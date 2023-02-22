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
    print_info "Start compressing.";
    tar -C $CIRRUS_WORKING_DIR/build/install-root/usr/local/Applications --exclude "*.DS_Store" -cvJf valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz Valentina.app/;
    check_failure "Unable to create an archive for Valentina bundle.";

    if [[ "$MULTI_BUNDLE" == "true" ]]; then
      tar -C $CIRRUS_WORKING_DIR/build/install-root/usr/local/Applications --exclude "*.DS_Store" -cvJf tape-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz Tape.app/;
      check_failure "Unable to create an archive for Tape bundle.";

      tar -C $CIRRUS_WORKING_DIR/build/install-root/usr/local/Applications --exclude "*.DS_Store" -cvJf puzzle-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz Puzzle.app/;
      check_failure "Unable to create an archive for Puzzle bundle.";
    fi

    print_info "Start uploading.";
    if [[ "$MULTI_BUNDLE" == "false" ]]; then
      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz";
      check_failure "Unable to upload Valentina bundle.";
    else
      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz";
      check_failure "Unable to upload Valentina bundle.";

      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/tape-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}/tape-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz";
    check_failure "Unable to upload Tape bundle.";

      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/puzzle-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}/puzzle-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.tar.xz";
    check_failure "Unable to upload Tape bundle.";
    fi

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
