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
    $CIRRUS_WORKING_DIR/scripts/macfixqtdylibrpath.py $CIRRUS_WORKING_DIR/build/src/app/valentina/bin/Valentina.app;
    check_failure "Unable to patch the app bundle.";

    print_info "Start compressing.";
    tar -C $CIRRUS_WORKING_DIR/build/src/app/valentina/bin --exclude "*.DS_Store" -cvJf valentina-${PLATFORM}-${CIRRUS_CHANGE_IN_REPO}.tar.xz Valentina.app/;
    check_failure "Unable to create an archive.";

    print_info "Start uploading.";
    python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/valentina-${PLATFORM}-${CIRRUS_CHANGE_IN_REPO}.tar.xz "/0.7.x/Mac OS X/valentina-${PLATFORM}-${CIRRUS_CHANGE_IN_REPO}.tar.xz";
    check_failure "Unable to upload.";

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
