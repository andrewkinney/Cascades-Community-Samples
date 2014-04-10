/*
 * Copyright (c) 2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>

#include "HubAccount.hpp"

#include <QDebug>

HubAccount::HubAccount(UDSUtil* udsUtil, HubCache* hubCache) : _udsUtil(udsUtil), _hubCache(hubCache)
{
	qDebug()  << "HubAccount::HubAccount " << udsUtil;

	_accountId = 0;
	_supportsCompose = false;
    _supportsMarkRead = false;
    _supportsMarkUnread = false;
	_initialized = false;
	_categoriesInitialized = false;
}

HubAccount::~HubAccount() {
	// TODO Auto-generated destructor stub
}

QVariantMap HubAccount::categories()
{
	return _hubCache->categories();
}

void HubAccount::initialize()
{
    qDebug()  << "HubAccount::initialize " << _initialized;

    if (!_initialized) {
        int retVal = -1;

        _accountId = _hubCache->accountId();

        if (_hubCache->accountId() < 0) {
            _accountId = _udsUtil->addAccount(_name, _displayName, _serverName, _cardTarget,
                                            _iconFilename, _lockedIconFilename, _composeIconFilename,
                                            _description,  _supportsCompose, UDS_ACCOUNT_TYPE_OTHER);

            if (_accountId > 0) {
                qDebug() << "HubAccount::initialize: addAccount succeeded for account " << _accountId << " name: " << _name << "\n";

                _hubCache->setAccountId(_accountId);
                _hubCache->setAccountName(_name);
            } else {
                qDebug() << "HubAccount::initialize: addAccount failed for account name: " << _name << "\n";
            }

            if (_supportsCompose) {
                int retVal = _udsUtil->addAccountAction(_accountId, QString("bb.action.COMPOSE"), QString(tr("Compose")),
                        _cardTarget, QString("application"), _itemComposeIconFilename, _itemMimeType, UDS_PLACEMENT_BAR);
                if (retVal != 0) {
                    qDebug() << "HubAccount::initialize: addAccountActionData: bb.action.COMPOSE : " << " retval: " << retVal;
                }
            }

            if (_supportsMarkRead) {
                retVal = _udsUtil->addItemAction(_accountId, QString("bb.action.MARKREAD"), QString(tr("Mark Read")),
                        _headlessTarget, QString("application.headless"), _markReadActionIconFilename, _itemMimeType, UDS_PLACEMENT_OVERFLOW);
                if (retVal != 0) {
                    qDebug() << "HubAccount::addHubItem: addItmActionData: addItmAction: bb.action.MARKREAD : " << " retval: " << retVal;
                }
            }

            if (_supportsMarkUnread) {
                retVal = _udsUtil->addItemAction(_accountId, QString("bb.action.MARKUNREAD"), QString(tr("Mark Unread")),
                        _headlessTarget, QString("application.headless"), _markUnreadActionIconFilename, _itemMimeType, UDS_PLACEMENT_OVERFLOW);
                if (retVal != 0) {
                    qDebug() << "HubAccount::addHubItem: addItmActionData: addItmAction: bb.action.MARKUNREAD : " << " retval: " << retVal;
                }
            }
        } else {
            QString accountName = _hubCache->accountName();

            _udsUtil->restoreNextIds(_accountId+1, _hubCache->lastCategoryId()+1, _hubCache->lastItemId()+1);
        }

        _initialized = true;
    }
}

void HubAccount::initializeCategories(QStringList newCategories)
{
    qDebug()  << "HubAccount::initializeCategories " << _categoriesInitialized;

    if (!_categoriesInitialized) {
        qint64 retVal = -1;

        if (_hubCache->categories().size() == 0) {
            QVariantMap categories;

            for(int index = 0; index < newCategories.size(); index++) {
                retVal = _udsUtil->addCategory(_accountId, newCategories[index]);
                if (retVal == -1) {
                    qDebug() << "HubAccount::initializeCategories: add category failed for: " << newCategories[index];
                    break;
                }

                categories[newCategories[index]] = QVariant(retVal);
            }

            if (retVal > 0) {
                _hubCache->setCategories(categories);
            }
        }

        QVariantList items = _hubCache->items();

        _categoriesInitialized = true;
    }
}

bool HubAccount::remove() {

    return _udsUtil->removeAccount(_accountId);

    _accountId = 0;
}

QVariant* HubAccount::getHubItem(qint64 categoryId, qint64 itemId)
{
    QVariant* item = _hubCache->getItem(categoryId,itemId);
    if (item) {
        QVariantMap itemMap = (*item).toMap();
    }

    return item;
}

QVariant* HubAccount::getHubItemBySyncID(qint64 categoryId, QString syncId)
{
    QVariant* item = _hubCache->getItemBySyncID(categoryId,syncId);
    if (item) {
        QVariantMap itemMap = (*item).toMap();
    }

    return item;
}

QVariantList HubAccount::items()
{
    return _hubCache->items();
}

bool HubAccount::addHubItem(qint64 categoryId, QVariantMap &itemMap, QString name, QString subject, qint64 timestamp, QString itemSyncId,  QString itemUserData, QString itemExtendedData, bool notify)
{
    qint64 retVal = 0;
    bool    itemRead = false;
    int itemContextState = 1;

	qDebug() << "add hub item: " << timestamp << " - " << name << " - " << subject;

    retVal = _udsUtil->addItem(_accountId, categoryId, itemMap, name, subject, _itemMimeType, _itemUnreadIconFilename,
                                itemRead, itemSyncId, itemUserData, itemExtendedData, timestamp, itemContextState, notify);

    if (retVal <= 0) {
        qDebug() << "HubAccount::addHubItem: addItem failed for item: " << name << ", category: " << categoryId << ", account: "<< _accountId << ", retVal: "<< retVal << "\n";
    } else {
        _hubCache->addItem(itemMap);
    }

    return (retVal > 0);
}

bool HubAccount::updateHubItem(qint64 categoryId, qint64 itemId, QVariantMap &itemMap, bool notify)
{
    qint64 retVal = 0;
    int itemContextState = 1;

    qDebug() << "update hub item: " << categoryId << " : " <<  itemId << " : " <<  itemMap << " : " <<  notify;

    if ((itemMap["readCount"].toInt() > 0)) {
        retVal = _udsUtil->updateItem(_accountId, categoryId, itemMap, QString::number(itemId), itemMap["name"].toString(), itemMap["description"].toString(), _itemMimeType, _itemReadIconFilename,
                (itemMap["readCount"].toInt() > 0), itemMap["syncId"].toString(), itemMap["userData"].toString(), itemMap["extendedData"].toString(), itemMap["timestamp"].toLongLong(), itemContextState, notify);
    } else {
        retVal = _udsUtil->updateItem(_accountId, categoryId, itemMap, QString::number(itemId), itemMap["name"].toString(), itemMap["description"].toString(), _itemMimeType, _itemUnreadIconFilename,
                (itemMap["readCount"].toInt() > 0), itemMap["syncId"].toString(), itemMap["userData"].toString(), itemMap["extendedData"].toString(), itemMap["timestamp"].toLongLong(), itemContextState, notify);
    }

    if (retVal <= 0) {
        qDebug() << "HubAccount::updateHubItem: updateItem failed for item: " << itemId << ", category: " << categoryId << ", account: "<< _accountId << ", retVal: "<< retVal << "\n";
    } else {
        _hubCache->updateItem(itemId, itemMap);
    }

    return (retVal > 0);
}

bool HubAccount::removeHubItem(qint64 categoryId, qint64 itemId)
{
    qint64 retVal = 0;

    qDebug() << "remove hub item: " << categoryId << " : " <<  itemId;

    retVal = _udsUtil->removeItem(_accountId, categoryId, QString::number(itemId));
    if (retVal <= 0) {
        qDebug() << "HubAccount::removeHubItem: removeItem failed for item: " << categoryId << " : " <<  itemId << ", retVal: "<< retVal << "\n";
    } else {
        _hubCache->removeItem(itemId);
    }

    return (retVal > 0);
}

void HubAccount::repopulateHub()
{
    QVariantList items = _hubCache->items();

    if (items.size() > 0) {
        for(int index = 0; index < items.size(); index++) {
            QVariantMap itemMap = items.at(index).toMap();

            if (itemMap["accountId"].toLongLong() ==  _accountId) {
                addHubItem(itemMap["categoryId"].toLongLong(), itemMap, itemMap["name"].toString(), itemMap["description"].toString(), itemMap["timestamp"].toLongLong(), itemMap["syncId"].toString(), itemMap["userData"].toString(), itemMap["extendedData"].toString(), false);
            }
        }
    }
}

