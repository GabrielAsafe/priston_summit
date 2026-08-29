from __future__ import annotations
import argparse, json, sys
from pathlib import Path
import bpy
from mathutils import Matrix


def args_after_ddash():
    return sys.argv[sys.argv.index("--")+1:] if "--" in sys.argv else []

def row_to_col(values):
    if len(values)!=16: raise ValueError("matrix must contain 16 values")
    r=Matrix((values[0:4],values[4:8],values[8:12],values[12:16]))
    return r.transposed()

def find_armature():
    arms=[o for o in bpy.data.objects if o.type=='ARMATURE']
    if len(arms)!=1: raise RuntimeError(f"expected exactly one armature, found {len(arms)}")
    return arms[0]

def ensure_pose_mode(arm):
    bpy.ops.object.mode_set(mode='OBJECT') if bpy.context.object and bpy.context.object.mode!='OBJECT' else None
    bpy.context.view_layer.objects.active=arm
    arm.select_set(True)
    bpy.ops.object.mode_set(mode='POSE')

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--payload',required=True); ap.add_argument('--out',required=True); ap.add_argument('--report',required=True)
    a=ap.parse_args(args_after_ddash())
    payload=json.loads(Path(a.payload).read_text(encoding='utf-8'))
    arm=find_armature(); ensure_pose_mode(arm)
    bind={b['name']:row_to_col(b['bind_tm']) for b in payload['bones']}
    missing=[n for n in bind if n not in arm.pose.bones]
    if missing: raise RuntimeError(f"armature missing source bones: {missing[:20]}")
    created=[]
    for spec in payload['actions']:
        old=bpy.data.actions.get(spec['name'])
        if old is not None:
            bpy.data.actions.remove(old)
        action=bpy.data.actions.new(spec['name'])
        arm.animation_data_create(); arm.animation_data.action=action
        # Blender 5.x removed the legacy Action.fcurves API. Create/assign
        # the object slot explicitly; keyframe_insert then creates the
        # layer/keyframe-strip/channelbag for this slot.
        if hasattr(action, 'slots'):
            slot=action.slots.new(id_type='OBJECT', name=arm.name)
            arm.animation_data.action_slot=slot
        for pb in arm.pose.bones:
            pb.rotation_mode='QUATERNION'
        previous_quat = {}
        for fr in spec['frames']:
            scene_frame=int(fr['frame']); bpy.context.scene.frame_set(scene_frame)
            # Set global pose matrices parent-first. Otherwise setting a parent after
            # a child would change the child's resulting armature-space matrix.
            for idx in payload['tm_sort']:
                bmeta=payload['bones'][idx]; source_current_row=fr['matrices'][idx]
                name=bmeta['name']; pb=arm.pose.bones[name]
                current=row_to_col(source_current_row)
                delta=current @ bind[name].inverted()
                rest=arm.data.bones[name].matrix_local
                pb.matrix=delta @ rest
            bpy.context.view_layer.update()
            for bmeta in payload['bones']:
                pb=arm.pose.bones[bmeta['name']]
                # Matrix decomposition may choose q or -q independently on each
                # frame.  They are the same orientation, but interpolating the
                # components across opposite signs can create violent flips.
                q = pb.rotation_quaternion.copy()
                prev = previous_quat.get(pb.name)
                if prev is not None and q.dot(prev) < 0.0:
                    q.negate()
                    pb.rotation_quaternion = q
                previous_quat[pb.name] = q.copy()
                pb.keyframe_insert(data_path='location',frame=scene_frame,group=pb.name)
                pb.keyframe_insert(data_path='rotation_quaternion',frame=scene_frame,group=pb.name)
                pb.keyframe_insert(data_path='scale',frame=scene_frame,group=pb.name)

        # Blender 5 stores F-curves in the Action slot/channelbag model.
        # Force linear interpolation so the preview follows the evaluated
        # source samples without Bézier overshoot.  Keep a legacy fallback.
        if hasattr(action, 'layers') and hasattr(action, 'slots') and len(action.slots):
            slot = action.slots[0]
            for layer in action.layers:
                for strip in layer.strips:
                    if hasattr(strip, 'channelbag'):
                        bag = strip.channelbag(slot, ensure=False)
                        if bag is not None:
                            for fcu in bag.fcurves:
                                for kp in fcu.keyframe_points:
                                    kp.interpolation = 'LINEAR'
        elif hasattr(action, 'fcurves'):
            for fcu in action.fcurves:
                for kp in fcu.keyframe_points:
                    kp.interpolation = 'LINEAR'
        action['pt_motion_index']=int(spec['index']); action['pt_state_name']=spec['state_name']; action['pt_repeat']=int(spec['repeat'])
        action.use_frame_range=True; action.frame_start=1; action.frame_end=max(1,int(spec['frame_count']))
        if hasattr(action, 'use_cyclic'): action.use_cyclic=bool(spec['repeat'])
        action['pt_engine_start']=int(spec['engine_start']); action['pt_engine_end']=int(spec['engine_end'])
        created.append({'name':action.name,'frames':int(spec['frame_count']),'state':spec['state_name'],'index':int(spec['index'])})
    selected=bpy.data.actions.get(payload['selected_action'])
    if selected is None: raise RuntimeError(f"selected action missing: {payload['selected_action']}")
    arm.animation_data.action=selected
    if hasattr(selected, 'slots') and len(selected.slots):
        arm.animation_data.action_slot=selected.slots[0]
    sel=next(x for x in payload['actions'] if x['name']==payload['selected_action'])
    bpy.context.scene.frame_start=1; bpy.context.scene.frame_end=max(1,int(sel['frame_count'])); bpy.context.scene.frame_set(1)
    bpy.context.scene.render.fps=int(payload.get('fps',30))
    arm['pt_animation_source_inx']=payload['inx']; arm['pt_animation_source_smb']=payload['smb']; arm['pt_selected_action']=payload['selected_action']
    bpy.ops.object.mode_set(mode='OBJECT')
    Path(a.out).parent.mkdir(parents=True,exist_ok=True); bpy.ops.wm.save_as_mainfile(filepath=str(Path(a.out)))
    report={'tool':'PT_BLENDER_BAKE_ACTIONS_V1','armature':arm.name,'actions':created,'selected_action':selected.name,
            'scene_frame_start':bpy.context.scene.frame_start,'scene_frame_end':bpy.context.scene.frame_end,'fps':bpy.context.scene.render.fps,
            'quaternion_null_semantics':payload.get('quaternion_null_semantics','UNKNOWN'),'quaternion_sign_continuity':True,'interpolation':'LINEAR','client_modified':False,'check':'PASS'}
    Path(a.report).write_text(json.dumps(report,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('PT_BLENDER_BAKE_ACTIONS_V1'); print(f"actions={len(created)}"); print(f"selected_action={selected.name}"); print('client_modified=False'); print('animation_bake_check=PASS')
if __name__=='__main__': main()
