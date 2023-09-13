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
    print_info "Start labeling.";
    if [[ "$MULTI_BUNDLE" == "false" ]]; then
      mv $CIRRUS_WORKING_DIR/build/install-root/share/valentina.dmg $CIRRUS_WORKING_DIR/build/install-root/share/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg;
    else
      mv $CIRRUS_WORKING_DIR/build/install-root/share/valentina.dmg $CIRRUS_WORKING_DIR/build/install-root/share/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg;
    fi
    check_failure "Unable to label Valentina DMG."; 

    print_info "Start cleaning.";
    python3 $CIRRUS_WORKING_DIR/scripts/deploy.py clean $ACCESS_TOKEN;
    check_failure "Unable to clean stale artifacts.";

    print_info "Start uploading.";
    if [[ "$MULTI_BUNDLE" == "false" ]]; then
      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/build/install-root/share/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg";
    else
      python3 $CIRRUS_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $CIRRUS_WORKING_DIR/build/install-root/share/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg "/0.7.x/Mac OS X/valentina-${PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg";
    fi
    check_failure "Unable to upload Valentina DMG.";

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
