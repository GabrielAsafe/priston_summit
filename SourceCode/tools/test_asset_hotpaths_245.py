#!/usr/bin/env python3
"""Regression and fail-closed checks for PATCH 245 source and model census."""
import argparse
import json
from pathlib import Path
import tempfile
import unittest
from audit_model_caches_245 import audit

class TestAssetHotpaths245(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.root = Path(cls.source_code)

    def source(self, path):
        return (self.root/path).read_bytes().decode('latin-1')

    def test_lua_cache_keeps_execution_each_call(self):
        cpp = self.source('SrcGame/src/systems/rendering/effects/legacy_new/script/PiScript.cpp')
        self.assertIn('lua_dump(m_pLuaState', cpp)
        self.assertIn('luaL_loadbuffer(m_pLuaState, bytes.data(), bytes.size()', cpp)
        self.assertEqual(cpp.count('if (lua_pcall(m_pLuaState, 0, 0, 0))'), 1)
        self.assertIn('GetFileAttributesExA(fileName', cpp)
        self.assertIn('unchangedDuringRead', cpp)
        self.assertIn('SUMMIT_LUA_MAX_CACHE', cpp)
        self.assertIn('m_cachedScripts.erase', cpp)

    def test_particle_cache_avoids_duplicate_open(self):
        s = self.source('Shared/rendering/effects/particles/HoNewParticleMgrLegacyCommon.inl')
        self.assertLess(s.index('if (FindScript(existingKey))'), s.index('fp = fopen(scr, "rb")'))

    def test_effect_models_guarded(self):
        s = self.source('Shared/rendering/effects/legacy_new/HoEffectResMgrLegacyCommon.inl')
        self.assertIn('meshName.size() >= 128u', s)
        self.assertIn('boneName.size() >= 128u', s)
        self.assertIn('smASE_SetPhysique(NULL); // Paired-bone', s)
        a = self.source('Shared/rendering/effects/legacy_sin/AssaUtil_legacy_client.inl')
        self.assertIn('if (!AssaRes[index].res)', a)
        self.assertIn('if (index == 0)', a)

    def test_census_safe_and_stale(self):
        with tempfile.TemporaryDirectory() as t:
            root = Path(t)
            client = root/'cliente'; client.mkdir(); (client/'Game.exe').write_bytes(b'game')
            effects = client/'Effect'; effects.mkdir()
            (effects/'one.ASE').write_text('mesh')
            (effects/'one.smd').write_bytes(b'cache')
            (effects/'one.smd').touch() # fresh mtime
            (effects/'two.ase').write_text('mesh')
            (effects/'Two.ASE').write_text('mesh') # casefold collision
            report = audit(client, root/'out.json')
            self.assertEqual(report['counts']['sources_ase'], 3)
            self.assertEqual(report['counts']['cache_smd_fresh'], 1)
            self.assertEqual(report['counts']['ambiguous_case'], 2)
            self.assertEqual(report['counts']['cache_smb_missing'], 1)
            self.assertFalse((effects/'one.smb').exists())

    def test_census_rejects_output_inside_client(self):
        with tempfile.TemporaryDirectory() as t:
            c = Path(t); (c/'Game.exe').write_bytes(b'game')
            with self.assertRaises(ValueError):
                audit(c, c/'report.json')

if __name__ == '__main__':
    p = argparse.ArgumentParser(); p.add_argument('--source-code', default='.')
    args = p.parse_args()
    TestAssetHotpaths245.source_code = args.source_code
    unittest.main(argv=['test_asset_hotpaths_245'], verbosity=2)
