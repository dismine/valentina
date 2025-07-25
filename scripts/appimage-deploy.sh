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
    print_info "Start cleaning.";
    python3 $CIRRUS_WORKING_DIR/scripts/deploy.py clean $ACCESS_TOKEN;
    check_failure "Unable to clean stale artifacts.";

    print_info "Start uploading.";
    python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/Valentina-continuous-x86_64.AppImage "/1.1.x/Linux/valentina-${TARGET_PLATFORM}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.AppImage";
    check_failure "Unable to upload Valentina's AppImage.";

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
