// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/background_sync/background_sync_context.h"

#include <utility>

#include "base/bind.h"
#include "base/stl_util.h"
#include "content/browser/background_sync/background_sync_manager.h"
#include "content/browser/background_sync/background_sync_service_impl.h"
#include "content/browser/service_worker/service_worker_context_wrapper.h"
#include "content/public/browser/browser_thread.h"

namespace content {

BackgroundSyncContext::BackgroundSyncContext() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
}

BackgroundSyncContext::~BackgroundSyncContext() {
  DCHECK(!background_sync_manager_);
  DCHECK(services_.empty());
}

void BackgroundSyncContext::Init(
    const scoped_refptr<ServiceWorkerContextWrapper>& context) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&BackgroundSyncContext::CreateBackgroundSyncManager, this,
                 context));
}

void BackgroundSyncContext::Shutdown() {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&BackgroundSyncContext::ShutdownOnIO, this));
}

void BackgroundSyncContext::CreateService(
    mojo::InterfaceRequest<blink::mojom::BackgroundSyncService> request) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);

  BrowserThread::PostTask(
      BrowserThread::IO, FROM_HERE,
      base::Bind(&BackgroundSyncContext::CreateServiceOnIOThread, this,
                 base::Passed(&request)));
}

void BackgroundSyncContext::ServiceHadConnectionError(
    BackgroundSyncServiceImpl* service) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(base::ContainsValue(services_, service));

  services_.erase(service);
  delete service;
}

BackgroundSyncManager* BackgroundSyncContext::background_sync_manager() const {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  return background_sync_manager_.get();
}

void BackgroundSyncContext::set_background_sync_manager_for_testing(
    std::unique_ptr<BackgroundSyncManager> manager) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  background_sync_manager_ = std::move(manager);
}

void BackgroundSyncContext::CreateBackgroundSyncManager(
    scoped_refptr<ServiceWorkerContextWrapper> context) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(!background_sync_manager_);

  background_sync_manager_ = BackgroundSyncManager::Create(context);
}

void BackgroundSyncContext::CreateServiceOnIOThread(
    mojo::InterfaceRequest<blink::mojom::BackgroundSyncService> request) {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);
  DCHECK(background_sync_manager_);
  services_.insert(new BackgroundSyncServiceImpl(this, std::move(request)));
}

void BackgroundSyncContext::ShutdownOnIO() {
  DCHECK_CURRENTLY_ON(BrowserThread::IO);

  base::STLDeleteElements(&services_);
  background_sync_manager_.reset();
}

}  // namespace content
