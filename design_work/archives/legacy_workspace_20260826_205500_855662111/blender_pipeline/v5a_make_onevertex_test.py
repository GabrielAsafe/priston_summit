import bpy

obj = bpy.data.objects["MahC20"]

v = obj.data.vertices[0]

print("ANTES:", tuple(v.co))

v.co.x += 1.0

print("DEPOIS:", tuple(v.co))

bpy.ops.wm.save_as_mainfile(
    filepath="/home/gabriel/Área de Trabalho/summit/design_work/blender_pipeline/MfbC13_import_v4_onevertex.blend"
)

print("SALVO: MfbC13_import_v4_onevertex.blend")
