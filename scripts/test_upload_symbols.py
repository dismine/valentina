#!/usr/bin/env python3
"""Plain-assert checks for upload_symbols.py pure pieces. No network, no pytest, no boto3."""

import os
import sys
import zipfile

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import upload_symbols as us

SHA40 = "a" * 40


def expect_systemexit(fn, *args, **kwargs):
    try:
        fn(*args, **kwargs)
    except SystemExit:
        return
    raise AssertionError(f"expected SystemExit from {fn.__name__}")


def test_r2_extension_covers_all_platforms():
    assert us.R2_EXTENSION == {"linux": "debug", "windows": "pdb", "macos": "dsym.zip"}


def test_r2_symbol_key_uses_normalized_application_name():
    key = us.r2_symbol_key("windows", SHA40, "valentina")
    assert key == f"builds/{SHA40}/windows/valentina.pdb"


def test_r2_symbol_key_macos_uses_dsym_zip_extension():
    key = us.r2_symbol_key("macos", SHA40, "qmuparserlib")
    assert key == f"builds/{SHA40}/macos/qmuparserlib.dsym.zip"


def test_r2_symbol_store_config_returns_all_four_values():
    env = {
        "R2_ACCOUNT_ID": "acct",
        "R2_DEBUG_SYMBOLS_ACCESS_KEY_ID": "key",
        "R2_DEBUG_SYMBOLS_SECRET_ACCESS_KEY": "secret",
        "R2_DEBUG_SYMBOLS_BUCKET": "bucket",
    }
    assert us.r2_symbol_store_config(env) == env


def test_r2_symbol_store_config_rejects_missing_variable():
    env = {"R2_ACCOUNT_ID": "acct", "R2_DEBUG_SYMBOLS_BUCKET": "bucket"}
    expect_systemexit(us.r2_symbol_store_config, env)


def test_r2_symbol_store_config_rejects_empty_variable():
    env = {
        "R2_ACCOUNT_ID": "acct",
        "R2_DEBUG_SYMBOLS_ACCESS_KEY_ID": "",
        "R2_DEBUG_SYMBOLS_SECRET_ACCESS_KEY": "secret",
        "R2_DEBUG_SYMBOLS_BUCKET": "bucket",
    }
    expect_systemexit(us.r2_symbol_store_config, env)


def test_parse_args_requires_commit_sha():
    expect_systemexit(
        us.parse_args,
        [
            "--build-dir", ".", "--app-version", "1_0_0",
            "--git-hash", "gabc123", "--qt-version", "Qt_6_8",
        ],
    )


def test_parse_args_accepts_commit_sha():
    args = us.parse_args([
        "--build-dir", ".", "--app-version", "1_0_0",
        "--git-hash", "gabc123", "--qt-version", "Qt_6_8",
        "--commit-sha", SHA40,
    ])
    assert args.commit_sha == SHA40


def test_heavy_dependencies_are_not_imported_at_module_scope():
    # Neither boto3 nor symbol-upload's network calls happen at import time.
    assert "boto3" not in sys.modules


def test_upload_to_symbol_store_uploads_plain_file_by_key():
    calls = []

    class StubClient:
        def upload_file(self, path, bucket, key):
            calls.append((path, bucket, key))

    target = us.Target("valentina", "valentina.debug", False, "valentina.debug")
    us.upload_to_symbol_store(
        client=StubClient(), bucket="val-debug-symbols", target=target,
        artifact_path=us.Path("/tmp/does-not-matter/valentina.debug"),
        platform="linux", commit_sha=SHA40,
    )
    assert calls == [
        ("/tmp/does-not-matter/valentina.debug", "val-debug-symbols",
         f"builds/{SHA40}/linux/valentina.debug")
    ]


def test_upload_to_symbol_store_zips_directory_bundles():
    calls = []
    captured_zip_bytes = {}

    class StubClient:
        def upload_file(self, path, bucket, key):
            # Snapshot the zip's bytes now -- upload_to_symbol_store cleans up
            # its scratch directory (including this file) right after this call.
            captured_zip_bytes["data"] = us.Path(path).read_bytes()
            calls.append((path, bucket, key))

    import tempfile as _tempfile
    parent_dir = _tempfile.mkdtemp()
    bundle_name = "Valentina.app.dSYM"
    bundle_dir = us.Path(parent_dir) / bundle_name
    bundle_dir.mkdir()
    (bundle_dir / "Info.plist").write_text("placeholder")

    target = us.Target("valentina", bundle_name, True, bundle_name)
    us.upload_to_symbol_store(
        client=StubClient(), bucket="val-debug-symbols", target=target,
        artifact_path=bundle_dir,
        platform="macos", commit_sha=SHA40,
    )
    assert len(calls) == 1
    path, bucket, key = calls[0]
    assert path.endswith("valentina.dsym.zip")
    assert bucket == "val-debug-symbols"
    assert key == f"builds/{SHA40}/macos/valentina.dsym.zip"

    # The zip must preserve the bundle's own directory name as its top-level
    # entry (e.g. "Valentina.app.dSYM/Info.plist"), not just the bundle's
    # contents flattened to the zip root -- otherwise the DWARF data loses the
    # name that identifies which binary it belongs to.
    import io
    with zipfile.ZipFile(io.BytesIO(captured_zip_bytes["data"])) as zf:
        namelist = zf.namelist()
        assert namelist, "zip should not be empty"
        top_level_dirs = {name.split("/")[0] for name in namelist}
        assert top_level_dirs == {bundle_name}

    # The scratch directory used to build the zip must be cleaned up afterward.
    assert not us.Path(path).parent.exists()


def test_upload_to_symbol_store_wraps_client_errors_as_systemexit():
    class FailingClient:
        def upload_file(self, path, bucket, key):
            raise RuntimeError("network down")

    target = us.Target("valentina", "valentina.debug", False, "valentina.debug")
    expect_systemexit(
        us.upload_to_symbol_store,
        client=FailingClient(), bucket="b", target=target,
        artifact_path=us.Path("/tmp/x/valentina.debug"), platform="linux", commit_sha=SHA40,
    )


def test_upload_to_symbol_store_cleans_up_scratch_dir_on_upload_failure():
    seen_paths = []

    class FailingClient:
        def upload_file(self, path, bucket, key):
            seen_paths.append(path)
            raise RuntimeError("network down")

    import tempfile as _tempfile
    parent_dir = _tempfile.mkdtemp()
    bundle_name = "Valentina.app.dSYM"
    bundle_dir = us.Path(parent_dir) / bundle_name
    bundle_dir.mkdir()
    (bundle_dir / "Info.plist").write_text("placeholder")

    target = us.Target("valentina", bundle_name, True, bundle_name)
    expect_systemexit(
        us.upload_to_symbol_store,
        client=FailingClient(), bucket="b", target=target,
        artifact_path=bundle_dir, platform="macos", commit_sha=SHA40,
    )
    assert len(seen_paths) == 1
    assert not us.Path(seen_paths[0]).parent.exists()


def test_r2_symbol_key_rejects_unknown_platform():
    expect_systemexit(us.r2_symbol_key, "solaris", SHA40, "valentina")


def test_validate_commit_sha_accepts_full_40_char_hex_sha():
    us.validate_commit_sha(SHA40)  # must not raise
    us.validate_commit_sha("F4373acf9c1234567890ABCDEF1234567890abcd")  # mixed case OK


def test_validate_commit_sha_rejects_abbreviated_hash():
    expect_systemexit(us.validate_commit_sha, "gf4373acf9")


def test_validate_commit_sha_rejects_non_hex_characters():
    expect_systemexit(us.validate_commit_sha, "z" * 40)


def main():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    for test in tests:
        test()
        print(f"ok  {test.__name__}")
    print(f"\n{len(tests)} passed")


if __name__ == "__main__":
    main()
