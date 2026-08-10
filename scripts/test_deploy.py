#!/usr/bin/env python3
"""Plain-assert checks for deploy.py key derivation. No network, no pytest, no boto3."""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import deploy

# 1786372320 == 2026-08-10T14:32:00Z
TS = "1786372320"
SHA = "a1b2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0"


def expect_systemexit(fn, *args, **kwargs):
    try:
        fn(*args, **kwargs)
    except SystemExit:
        return
    raise AssertionError(f"expected SystemExit from {fn.__name__}")


def test_branch_root_maps_known_branches():
    assert deploy.branch_root("master") == "stable"
    assert deploy.branch_root("develop") == "edge"


def test_branch_root_rejects_unknown_branch():
    expect_systemexit(deploy.branch_root, "feature/nice-idea")


def test_build_folder_is_utc_minute_plus_sha12():
    assert deploy.format_build_folder(TS, SHA) == "20260810T1432-a1b2c3d4e5f6"


def test_sha_always_truncated_to_twelve():
    # The %h trap: git abbreviates to 9 locally and fewer on a shallow CI clone.
    # Whatever width arrives, the folder must use exactly 12.
    for width in (12, 20, 40):
        folder = deploy.format_build_folder(TS, SHA[:width])
        assert folder.split("-")[1] == "a1b2c3d4e5f6", folder
        assert len(folder.split("-")[1]) == 12, folder


def test_object_key_assembles_full_path():
    key = deploy.object_key("master", "windows", "valentina-setup.exe", TS, SHA)
    assert key == "stable/20260810T1432-a1b2c3d4e5f6/windows/valentina-setup.exe"


def test_object_key_rejects_unknown_platform():
    expect_systemexit(deploy.object_key, "master", "solaris", "x.tar", TS, SHA)


def test_r2_config_returns_all_four_values():
    env = {
        "R2_ACCOUNT_ID": "acct",
        "R2_ACCESS_KEY_ID": "key",
        "R2_SECRET_ACCESS_KEY": "secret",
        "R2_DEV_BUCKET": "bucket",
    }
    assert deploy.r2_config(env) == env


def test_r2_config_rejects_missing_variable():
    env = {"R2_ACCOUNT_ID": "acct", "R2_ACCESS_KEY_ID": "key", "R2_DEV_BUCKET": "bucket"}
    expect_systemexit(deploy.r2_config, env)


def test_r2_config_rejects_empty_variable():
    env = {
        "R2_ACCOUNT_ID": "acct",
        "R2_ACCESS_KEY_ID": "",
        "R2_SECRET_ACCESS_KEY": "secret",
        "R2_DEV_BUCKET": "bucket",
    }
    expect_systemexit(deploy.r2_config, env)


def test_heavy_dependencies_are_not_imported_at_module_scope():
    # Neither boto3 nor py7zr is installed on a developer checkout; both arrive
    # only via requirements-ci.txt on a runner. A module-scope import would make
    # this whole file unrunnable outside CI.
    assert "boto3" not in sys.modules
    assert "py7zr" not in sys.modules


def main():
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_")]
    for test in tests:
        test()
        print(f"ok  {test.__name__}")
    print(f"\n{len(tests)} passed")


if __name__ == "__main__":
    main()
