#!/usr/bin/env python3
"""PATCH 289: preview-only archival fallback without weakening native publishing."""
from pathlib import Path
from unittest import TestCase, main
from unittest.mock import patch
from types import SimpleNamespace
import hashlib, sys

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gallery_native_models_287 as gallery

GOOD_LIVE = {'verified':134, 'missing':0, 'complete':True}
GOOD_ARCHIVE = {'verified':134, 'expected':134, 'missing':0, 'complete':True,
                'read_only':True, 'deployment_allowed':False,
                'identity_changed':['server_sha256','client_sha256']}
DRIFT = 'client/server/converter version differs from verified staging'

class Gallery289(TestCase):
    def run_gate(self, strict, archival=None):
        def _strict(*a,**kw):
            if isinstance(strict,BaseException): raise strict
            return strict
        def _archive(*a,**kw):
            if isinstance(archival,BaseException): raise archival
            return archival
        modules = {
           'verify_native_model_stage_267':SimpleNamespace(verify_staging=_strict),
           'verify_archived_native_stage_279':SimpleNamespace(verify_archived_readonly=_archive)
        }
        with patch.dict(sys.modules,modules):
            return gallery.verify_gallery_readonly_289(Path('c'),Path('s'),Path('o'),Path('g'))

    def test_current_gate_preferred(self):
        self.assertEqual(self.run_gate(GOOD_LIVE, AssertionError('must not use archival')),
                         ('CURRENT_267', []))

    def test_executable_drift_only_uses_full_archival_verifier(self):
        self.assertEqual(self.run_gate(ValueError(DRIFT),GOOD_ARCHIVE),
                         ('ARCHIVAL_279', ['server_sha256','client_sha256']))

    def test_strict_other_error_not_overridden(self):
        with self.assertRaisesRegex(ValueError,'corrupt staged model'):
            self.run_gate(ValueError('corrupt staged model'),GOOD_ARCHIVE)

    def test_archival_corrupt_or_changed_ase_rejected(self):
        with self.assertRaisesRegex(ValueError,'source ASE changed'):
            self.run_gate(ValueError(DRIFT),ValueError('source ASE changed'))

    def test_archival_incomplete_refused(self):
        with self.assertRaisesRegex(ValueError,'incomplete'):
            self.run_gate(ValueError(DRIFT), {**GOOD_ARCHIVE, 'verified':133})

    def test_archival_cannot_claim_deploy_permission(self):
        with self.assertRaisesRegex(ValueError,'incomplete'):
            self.run_gate(ValueError(DRIFT),{**GOOD_ARCHIVE,'deployment_allowed':True})

    def test_archival_must_prove_identity_differs(self):
        with self.assertRaisesRegex(ValueError,'not proven'):
            self.run_gate(ValueError(DRIFT),{**GOOD_ARCHIVE,'identity_changed':[]})

    def test_archival_rejects_unknown_identity_field(self):
        with self.assertRaisesRegex(ValueError,'not proven'):
            self.run_gate(ValueError(DRIFT),{**GOOD_ARCHIVE,'identity_changed':['foo']})

    def test_archival_rejects_duplicate_identity_key(self):
        with self.assertRaisesRegex(ValueError,'not proven'):
            self.run_gate(ValueError(DRIFT),{**GOOD_ARCHIVE,'identity_changed':['client_sha256','client_sha256']})

    def test_no_automatic_install_from_gallery(self):
        code=Path(gallery.__file__).read_text()
        self.assertNotIn('shutil.copy2(',code)
        self.assertNotIn('git add',code)
        self.assertNotIn('canonical/',code)
        self.assertIn('deployment_allowed=0',code)

    def test_287_original_manifest_remains_hash_verified(self):
        code=Path(gallery.__file__).read_text()
        self.assertIn("digest(target) != item.get('sha256')",code)
        self.assertIn('validate_manifest(stage)',code)

    def test_preparation_disallows_auto_retry_for_non_identity_gate_failures(self):
        code=Path(gallery.__file__).read_text()
        self.assertIn("str(exc).strip() != 'client/server/converter version differs from verified staging'", code)

if __name__=='__main__': main(verbosity=2)
