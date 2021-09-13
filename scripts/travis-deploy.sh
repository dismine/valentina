print_error() {
    echo "[CI] ERROR: $1"
}


print_info() {
    echo "[CI] INFO: $1"
}


check_failure() {
    if [ $? -ne 0 ] ; then
        if [ -z $1 ] ; then
            print_error $1
        else
            print_error "Failure exit code is detected."
        fi
        exit 1
    fi
}

if [[ "$DEPLOY" == "true" ]]; then
    ../scripts/macfixqtdylibrpath.py $TRAVIS_BUILD_DIR/build/src/app/valentina/bin/Valentina.app;
    check_failure "Unable to patch the app bundle.";

    if [[ "$LEGACY" = false ]]; then
        legacy_suffix=""    
    else
        legacy_suffix="-legacy"
    fi

    print_info "Start compressing.";
    tar -C $TRAVIS_BUILD_DIR/build/src/app/valentina/bin --exclude "*.DS_Store" -cvJf valentina-osx-${XCODE}-${TRAVIS_COMMIT}${legacy_suffix}.tar.xz Valentina.app/;
    check_failure "Unable to create an archive.";

    print_info "Start uploading.";
    python3 $TRAVIS_BUILD_DIR/scripts/deploy.py upload $ACCESS_TOKEN $TRAVIS_BUILD_DIR/build/valentina-osx-${XCODE}-${TRAVIS_COMMIT}${legacy_suffix}.tar.xz "/0.7.x/Mac OS X/valentina-osx-${XCODE}-${TRAVIS_COMMIT}${legacy_suffix}.tar.xz";
    check_failure "Unable to upload.";

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
