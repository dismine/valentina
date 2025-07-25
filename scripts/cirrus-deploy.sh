print_error() {
    echo "[CI] ERROR: $1"
}


print_info() {
    echo "[CI] INFO: $1"
}


check_failure() {
    if [ $? -ne 0 ] ; then
        if [ -n "$1" ] ; then
            print_error "$1"
        else
            print_error "Failure exit code is detected."
        fi
        exit 1
    fi
}

if [[ "$DEPLOY" == "true" ]]; then
    print_info "Start labeling.";
    if [[ "$MULTI_BUNDLE" == "false" ]]; then
      mv $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina.dmg $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg;
    else
      mv $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina.dmg $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg;
    fi
    check_failure "Unable to label Valentina DMG."; 

    print_info "Start cleaning.";
    python3 $VALENTINA_WORKING_DIR/scripts/deploy.py clean $ACCESS_TOKEN;
    check_failure "Unable to clean stale artifacts.";

    print_info "Start uploading.";
    if [[ "$MULTI_BUNDLE" == "false" ]]; then
      python3 $VALENTINA_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg "/1.1.x/MacOS/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-${CIRRUS_CHANGE_IN_REPO}.dmg";
    else
      python3 $VALENTINA_WORKING_DIR/scripts/deploy.py upload $ACCESS_TOKEN $VALENTINA_WORKING_DIR/build/release/install-root/share/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg "/1.1.x/MacOS/valentina-${TARGET_PLATFORM}-${QT_VERSION}-${ARCH}-${CIRRUS_BRANCH}-multibundle-${CIRRUS_CHANGE_IN_REPO}.dmg";
    fi
    check_failure "Unable to upload Valentina DMG.";

    print_info "Successfully uploaded.";
else
    print_info "No deployment needed.";
fi
