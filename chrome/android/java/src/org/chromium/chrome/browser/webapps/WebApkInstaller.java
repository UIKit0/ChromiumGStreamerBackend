// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.webapps;

import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Looper;
import android.provider.Settings;

import org.chromium.base.ApplicationState;
import org.chromium.base.ApplicationStatus;
import org.chromium.base.ContextUtils;
import org.chromium.base.Log;
import org.chromium.base.annotations.CalledByNative;
import org.chromium.chrome.browser.ShortcutHelper;
import org.chromium.chrome.browser.banners.InstallerDelegate;

import java.io.File;

/**
 * Java counterpart to webapk_installer.h
 * Contains functionality to install / update WebAPKs.
 * This Java object is created by and owned by the native WebApkInstaller.
 */
public class WebApkInstaller {
    private static final String TAG = "WebApkInstaller";

    /** The WebAPK's package name. */
    private String mWebApkPackageName;

    /** Monitors for application state changes. */
    private ApplicationStatus.ApplicationStateListener mListener;

    /** Monitors installation progress. */
    private InstallerDelegate mInstallTask;

    /** Indicates whether to install or update a WebAPK. */
    private boolean mIsInstall;

    /** Weak pointer to the native WebApkInstaller. */
    private long mNativePointer;

    private WebApkInstaller(long nativePtr) {
        mNativePointer = nativePtr;
    }

    @CalledByNative
    private static WebApkInstaller create(long nativePtr) {
        return new WebApkInstaller(nativePtr);
    }

    @CalledByNative
    private void destroy() {
        ApplicationStatus.unregisterApplicationStateListener(mListener);
        mNativePointer = 0;
    }

    /**
     * Installs a WebAPK.
     * @param filePath File to install.
     * @param packageName Package name to install WebAPK at.
     * @return True if the install was started. A "true" return value does not guarantee that the
     *         install succeeds.
     */
    @CalledByNative
    private boolean installAsyncFromNative(String filePath, String packageName) {
        if (!installingFromUnknownSourcesAllowed()) {
            Log.e(TAG,
                    "WebAPK install failed because installation from unknown sources is disabled.");
            return false;
        }
        mIsInstall = true;
        return installDownloadedWebApk(filePath, packageName);
    }

    private boolean installDownloadedWebApk(String filePath, String packageName) {
        mWebApkPackageName = packageName;

        // Start monitoring the installation.
        PackageManager packageManager = ContextUtils.getApplicationContext().getPackageManager();
        mInstallTask = new InstallerDelegate(Looper.getMainLooper(), packageManager,
                createInstallerDelegateObserver(), mWebApkPackageName);
        mInstallTask.start();
        // Start monitoring the application state changes.
        mListener = createApplicationStateListener();
        ApplicationStatus.registerApplicationStateListener(mListener);

        Intent intent = new Intent(Intent.ACTION_VIEW);
        Uri fileUri = Uri.fromFile(new File(filePath));
        intent.setDataAndType(fileUri, "application/vnd.android.package-archive");
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        try {
            ContextUtils.getApplicationContext().startActivity(intent);
        } catch (ActivityNotFoundException e) {
            return false;
        }
        return true;
    }

    private InstallerDelegate.Observer createInstallerDelegateObserver() {
        return new InstallerDelegate.Observer() {
            @Override
            public void onInstallFinished(InstallerDelegate task, boolean success) {
                if (mInstallTask != task) return;
                onInstallFinishedInternal(success);
            }
        };
    }

    private void onInstallFinishedInternal(boolean success) {
        ApplicationStatus.unregisterApplicationStateListener(mListener);
        mInstallTask = null;
        if (mNativePointer != 0) {
            nativeOnInstallFinished(mNativePointer, success);
        }
        if (success && mIsInstall) {
            ShortcutHelper.addWebApkShortcut(ContextUtils.getApplicationContext(),
                    mWebApkPackageName);
        }
    }

    /**
     * Updates a WebAPK.
     * @param filePath File to update.
     * @param packageName Package name to update WebAPK at.
     * @return True if the update was started. A "true" return value does not guarantee that the
     *         update succeeds.
     */
    @CalledByNative
    private boolean updateAsyncFromNative(String filePath, String packageName) {
        mIsInstall = false;
        return installDownloadedWebApk(filePath, packageName);
    }

    /**
     * Returns whether the user has enabled installing apps from sources other than the Google Play
     * Store.
     */
    private static boolean installingFromUnknownSourcesAllowed() {
        Context context = ContextUtils.getApplicationContext();
        try {
            return Settings.Secure.getInt(
                           context.getContentResolver(), Settings.Secure.INSTALL_NON_MARKET_APPS)
                    == 1;
        } catch (Settings.SettingNotFoundException e) {
            return false;
        }
    }

    private ApplicationStatus.ApplicationStateListener createApplicationStateListener() {
        return new ApplicationStatus.ApplicationStateListener() {
            @Override
            public void onApplicationStateChange(int newState) {
                if (!ApplicationStatus.hasVisibleActivities()) return;
                /**
                 * Currently WebAPKs aren't installed by Play. A user can cancel the installation
                 * when the Android native installation dialog shows. The only way to detect the
                 * user cancelling the installation is to check whether the WebAPK is installed
                 * when Chrome is resumed. The monitoring of application state changes will be
                 * removed once WebAPKs are installed by Play.
                 */
                if (newState == ApplicationState.HAS_RUNNING_ACTIVITIES
                        && !isWebApkInstalled(mWebApkPackageName)) {
                    onInstallFinishedInternal(false);
                    return;
                }
            }
        };
    }

    private boolean isWebApkInstalled(String packageName) {
        PackageManager packageManager = ContextUtils.getApplicationContext().getPackageManager();
        return InstallerDelegate.isInstalled(packageManager, packageName);
    }

    private native void nativeOnInstallFinished(long nativeWebApkInstaller, boolean success);
}
