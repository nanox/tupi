/***************************************************************************
 *   Project TUPI: Magia 2D                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "ktcommandexecutor.h"

#include "ktserializer.h"
#include "ktscene.h"

#include "ktpathitem.h"
#include "ktrectitem.h"
#include "ktlineitem.h"
#include "ktellipseitem.h"
#include "ktitemconverter.h"
#include "ktsvg2qt.h"

#include "ktprojectrequest.h"
#include "ktrequestbuilder.h"
#include "ktitemfactory.h"
#include "ktprojectresponse.h"
#include "ktproxyitem.h"
#include "kttweenerstep.h"
#include "ktitemtweener.h"
#include "ktgraphicobject.h"
#include "ktlayer.h"
#include "ktbackground.h"

#include "tdebug.h"

#include <QGraphicsItem>

bool KTCommandExecutor::createItem(KTItemResponse *response)
{
    #ifdef K_DEBUG
           T_FUNCINFOX("items");
    #endif

    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    // int position = response->itemIndex();
    KTLibraryObject::Type type = response->itemType(); 
    QPointF point = response->position();
    KTProject::Mode mode = response->spaceMode();
    QString xml = response->arg().toString();

    KTScene *scene = m_project->scene(scenePosition);
  
    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    if (type == KTLibraryObject::Svg) {
                        KTSvgItem *svg = frame->createSvgItem(frame->svgItemsCount(), point, xml);
                        if (svg)
                            response->setItemIndex(frame->indexOf(svg));
                        else
                            return false;
                    } else {

                        tFatal() << "KTCommandExecutor::createItem() - XML: " << xml;
                        QGraphicsItem *item = frame->createItem(frame->graphicItemsCount(), point, xml);

                        if (item)
                            response->setItemIndex(frame->indexOf(item));
                        else
                            return false;
                    }

                    emit responsed(response);

                } else {
                    #ifdef K_DEBUG
                           tError() << "KTCommandExecutor::createItem() - Error: " << tr("Frame doesn't exists!");
                    #endif
                    return false;
                }
            } else {
                #ifdef K_DEBUG
                       tError() << "KTCommandExecutor::createItem() - Error: " << tr("Layer doesn't exists!");
                #endif
                return false;
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) {

            KTBackground *bg = scene->background();
            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    if (type == KTLibraryObject::Svg) {
                        KTSvgItem *svg = frame->createSvgItem(frame->svgItemsCount(), point, xml);
                        if (svg)
                            response->setItemIndex(frame->indexOf(svg));
                        else
                            return false;
                    } else {
                        QGraphicsItem *item = frame->createItem(frame->graphicItemsCount(), point, xml);
                        if (item)
                            response->setItemIndex(frame->indexOf(item));
                        else
                            return false;
                    }

                    emit responsed(response);
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::createItem() - mode invalid!";
            #endif

            return false;
        }
    } else {
        #ifdef K_DEBUG
               tError() << "KTCommandExecutor::createItem() - Error: " << tr("Scene doesn't exists!");
        #endif
        return false;
    }
    
    return true;
}

bool KTCommandExecutor::removeItem(KTItemResponse *response)
{
    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    KTLibraryObject::Type type = response->itemType();
    KTProject::Mode mode = response->spaceMode();

    KTScene *scene = m_project->scene(scenePosition);

    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);

            if (layer) {

                KTFrame *frame = layer->frame(framePosition);

                if (frame) {
                    if (type == KTLibraryObject::Svg) {
                        frame->removeSvgAt(response->itemIndex());

                        emit responsed(response);
                        return true;
                    } else {
                        KTGraphicObject *object = frame->graphic(response->itemIndex());
                        if (object) {
                            frame->removeGraphicAt(response->itemIndex());
                            if (object->hasTween()) {
                                tFatal() << "KTCommandExecutor::removeItem() - Remove tween from scene!";
                                scene->removeTweenObject(object);
                            }

                            emit responsed(response);
                            return true;

                        } else {
                            #ifdef K_DEBUG
                                   tError() << "KTCommandExecutor::removeItem() - Error: " << tr("Invalid object index") << response->itemIndex();
                            #endif
                            return false;
                        }
                    }
                }
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) {

            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {

                    if (type == KTLibraryObject::Svg) 
                        frame->removeSvgAt(response->itemIndex());
                    else
                         frame->removeGraphicAt(response->itemIndex());

                    emit responsed(response);
                    return true;
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::removeItem() - mode invalid!";
            #endif

            return false;
        }

    } else {
        #ifdef K_DEBUG
               tError() << "KTCommandExecutor::removeItem() - Error: " << tr("Scene doesn't exist!");
        #endif
        return false;
    }
    
    return false;
}

bool KTCommandExecutor::moveItem(KTItemResponse *response)
{
    #ifdef K_DEBUG
           T_FUNCINFOX("items");
    #endif

    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    int newPositon = response->arg().toInt();
    KTProject::Mode mode = response->spaceMode();

    if (response->mode() == KTProjectResponse::Undo) {
        position = newPositon;
        newPositon = response->itemIndex();
    }
    
    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {
            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    if (frame->moveItem(position, newPositon)) {
                        emit responsed(response);
                        return true;
                    }
                }
            }
        } else if (mode == KTProject::BACKGROUND_EDITION) {
            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    if (frame->moveItem(position, newPositon)) {
                        emit responsed(response);
                        return true;
                    }
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::moveItem() - mode invalid!";
            #endif

            return false;
        }
    }

    return false;
}


bool KTCommandExecutor::groupItems(KTItemResponse *response)
{
    #ifdef K_DEBUG
           T_FUNCINFOX("items");
    #endif
    
    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    KTProject::Mode mode = response->spaceMode();
    QString strList = response->arg().toString();
    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {
        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    QString::const_iterator itr = strList.constBegin();
                    QList<qreal> positions = KTSvg2Qt::parseNumbersList(++itr);
                    response->setItemIndex(frame->indexOf(frame->createItemGroupAt(position, positions)));
                
                    emit responsed(response);
                    return true;
                }
            }
        } else if (mode == KTProject::BACKGROUND_EDITION) {
                   KTBackground *bg = scene->background();

                   if (bg) {
                       KTFrame *frame = bg->frame();
                       if (frame) {
                           QString::const_iterator itr = strList.constBegin();
                           QList<qreal> positions = KTSvg2Qt::parseNumbersList(++itr);
                           response->setItemIndex(frame->indexOf(frame->createItemGroupAt(position, positions)));

                           emit responsed(response);
                           return true;
                       }
                   }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::groupItems() - mode invalid!";
            #endif

            return false;
        }
    }
    
    return false;
}

bool KTCommandExecutor::ungroupItems(KTItemResponse *response)
{
    #ifdef K_DEBUG
           T_FUNCINFOX("items");
    #endif
    
    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    KTProject::Mode mode = response->spaceMode();
    
    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    QString strItems = "";
                    QList<QGraphicsItem *> items = frame->destroyItemGroup(position);
                    foreach (QGraphicsItem *item, items) {
                             if (frame->indexOf(item) != -1) {
                                 if (strItems.isEmpty()) {
                                     strItems += "("+ QString::number(frame->indexOf(item));
                                 } else {
                                     strItems += " , "+ QString::number(frame->indexOf(item));
                                 }
                             }
                    }
                    strItems+= ")";
                    response->setArg(strItems);
                    emit responsed(response);

                    return true;
                }
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) {

            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    QString strItems = "";
                    QList<QGraphicsItem *> items = frame->destroyItemGroup(position);
                    foreach (QGraphicsItem *item, items) {
                             if (frame->indexOf(item) != -1) {
                                 if (strItems.isEmpty()) {
                                     strItems += "("+ QString::number(frame->indexOf(item));
                                 } else {
                                     strItems += " , "+ QString::number(frame->indexOf(item));
                                 }
                             }
                    }
                    strItems+= ")";
                    response->setArg(strItems);
                    emit responsed(response);

                    return true;
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::ungroupItems() - mode invalid!";
            #endif

            return false;
        }
    }

    return false;
}

static QGraphicsItem * convert(QGraphicsItem *item, int toType)
{
    switch (toType) {
            case KTPathItem::Type: // Path
            {
                 KTPathItem *path = KTItemConverter::convertToPath(item);
                 return path;
            }
            break;
            case KTRectItem::Type: // Rect
            {
                 KTRectItem *rect = KTItemConverter::convertToRect(item);
                 return rect;
            }
            break;
            case KTEllipseItem::Type: // Ellipse
            {
                 KTEllipseItem *ellipse = KTItemConverter::convertToEllipse(item);
                 return ellipse;
            }
            break;
            case KTProxyItem::Type:
            {
                 return new KTProxyItem(item);
            }
            break;
            case KTLineItem::Type:
            {
                 return KTItemConverter::convertToLine(item);
            }
            break;
            default:
            {
            #ifdef K_DEBUG
                tWarning() << "unknown item " << toType ;
            #endif
            }
            break;
    }

    return 0;
}

bool KTCommandExecutor::convertItem(KTItemResponse *response)
{
    #ifdef K_DEBUG
        T_FUNCINFOX("items");
    #endif

    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    KTProject::Mode mode = response->spaceMode();
    int toType = response->arg().toInt();
    
    KTScene *scene = m_project->scene(scenePosition);

    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    QGraphicsItem *item = frame->item(position);
                    if (item) {
                        tDebug("items") << item->type();
                    
                        if (toType == item->type()) 
                            return false;
                    
                        QGraphicsItem * itemConverted = convert(item, toType);
                    
                        if (itemConverted) {
                            // scene->removeItem(item); // FIXME?
                            // scene->addItem(itemConverted); // FIXME?
                            itemConverted->setZValue(item->zValue());
                            frame->replaceItem(position, itemConverted);
                        
                            response->setArg(QString::number(item->type()));
                            emit responsed(response);

                            return true;
                        }
                    }
                }
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) {

            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    QGraphicsItem *item = frame->item(position);
                    if (item) {
                        tDebug("items") << item->type();

                        if (toType == item->type())
                            return false;

                        QGraphicsItem * itemConverted = convert(item, toType);

                        if (itemConverted) {
                            itemConverted->setZValue(item->zValue());
                            frame->replaceItem(position, itemConverted);

                            response->setArg(QString::number(item->type()));
                            emit responsed(response);

                            return true;
                        }
                    }
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::convertItem() - mode invalid!";
            #endif

            return false;
        }
    }

    return false;
}

bool KTCommandExecutor::transformItem(KTItemResponse *response)
{
    #ifdef K_DEBUG
           T_FUNCINFOX("items");
    #endif

    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    KTProject::Mode mode = response->spaceMode();
    KTLibraryObject::Type type = response->itemType();
    QString xml = response->arg().toString();
    
    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    QGraphicsItem *item;
                    if (type == KTLibraryObject::Svg)
                        item = frame->svg(position);
                    else
                        item = frame->item(position);

                    if (item) {
                        QDomDocument orig;
                        orig.appendChild(KTSerializer::properties(item, orig));
                        QString current = orig.toString();

                        QDomDocument doc;
                        doc.setContent(xml);
                        KTSerializer::loadProperties(item, doc.documentElement());
                   
                        response->setArg(current); 
                        emit responsed(response);
                    
                        return true;
                    } 
                }
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) {

            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    QGraphicsItem *item;
                    if (type == KTLibraryObject::Svg)
                        item = frame->svg(position);
                    else
                        item = frame->item(position);

                    if (item) {
                        QDomDocument orig;
                        orig.appendChild(KTSerializer::properties(item, orig));
                        QString current = orig.toString();

                        QDomDocument doc;
                        doc.setContent(xml);
                        KTSerializer::loadProperties(item, doc.documentElement());

                        response->setArg(current);
                        emit responsed(response);

                        return true;
                    }
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::transformItem() - mode invalid!";
            #endif

            return false;
        }
    }
    
    return false;
}

bool KTCommandExecutor::setPathItem(KTItemResponse *response)
{
    /*
    #ifdef K_DEBUG
        T_FUNCINFOX("items");
    #endif
    */
    
    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    int position = response->itemIndex();
    KTProject::Mode mode = response->spaceMode();
    QString xml = response->arg().toString();

    /*
    #ifdef K_DEBUG
        SHOW_VAR(xml);
    #endif
    */

    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {

        if (mode == KTProject::FRAMES_EDITION) {

            KTLayer *layer = scene->layer(layerPosition);
            if (layer) {
                KTFrame *frame = layer->frame(framePosition);
                if (frame) {
                    QGraphicsItem *item = frame->item(position);
                    if (item) {
                        if (qgraphicsitem_cast<QGraphicsPathItem *>(item)) {
                            QDomDocument orig;
                        
                            if (KTPathItem *ktpath = qgraphicsitem_cast<KTPathItem *>(item))
                                orig.appendChild(ktpath->toXml(orig));

                            QString current = orig.toString();
                            /*
                            #ifdef K_DEBUG
                                   SHOW_VAR(current);
                            #endif
                            */
                        
                            QDomDocument doc;
                            doc.setContent(xml);
                            KTItemFactory factory;
                            factory.loadItem(item, xml);
                            emit responsed(response);
                            response->setArg(current);

                           return true;
                       }
                    }
                }
            }

        } else if (mode == KTProject::BACKGROUND_EDITION) { 

            KTBackground *bg = scene->background();

            if (bg) {
                KTFrame *frame = bg->frame();
                if (frame) {
                    QGraphicsItem *item = frame->item(position);
                    if (item) {
                        if (qgraphicsitem_cast<QGraphicsPathItem *>(item)) {
                            QDomDocument orig;

                            if (KTPathItem *ktpath = qgraphicsitem_cast<KTPathItem *>(item))
                                orig.appendChild(ktpath->toXml(orig));

                            QString current = orig.toString();
                            /*
                            #ifdef K_DEBUG
                                   SHOW_VAR(current);
                            #endif
                            */

                            QDomDocument doc;
                            doc.setContent(xml);
                            KTItemFactory factory;
                            factory.loadItem(item, xml);
                            emit responsed(response);
                            response->setArg(current);

                           return true;
                       }
                    }
                }
            }
        } else {
            #ifdef K_DEBUG
                   tError() << "KTCommandExecutor::setPathItem() - mode invalid!";
            #endif

            return false;
        }
    }

    return false;
}


bool KTCommandExecutor::setTween(KTItemResponse *response)
{
    #ifdef K_DEBUG
        T_FUNCINFO;
        SHOW_VAR(response);
    #endif

    int scenePosition = response->sceneIndex();
    int layerPosition = response->layerIndex();
    int framePosition = response->frameIndex();
    KTLibraryObject::Type itemType = response->itemType();
    int position = response->itemIndex();
    
    QString xml = response->arg().toString();
    // tFatal() << "KTCommandExecutor::setTween() - xml: ";
    // tFatal() << xml;

    KTScene *scene = m_project->scene(scenePosition);
    
    if (scene) {

        KTLayer *layer = scene->layer(layerPosition);

        if (layer) {

            KTFrame *frame = layer->frame(framePosition);

            if (frame) {

                KTItemTweener *tween = new KTItemTweener();
                tween->fromXml(xml);

                if (itemType == KTLibraryObject::Item) {

                    KTGraphicObject *object = frame->graphic(position);
                    if (object == 0) {
                        #ifdef K_DEBUG
                               tFatal() << "KTCommandExecutor::setTween() - Invalid graphic index: " << position;
                        #endif
                        return false;
                    } else {
                        object->setTween(tween);
                        scene->addTweenObject(object);
                    }

                } else {
                    KTSvgItem *object = frame->svg(position); 
                    if (object == 0) {
                        #ifdef K_DEBUG
                               tFatal() << "KTCommandExecutor::setTween() - Invalid svg index: " << position;
                        #endif
                        return false;
                    } else {
                        object->setTween(tween);
                        scene->addTweenObject(object);
                    }
                }

                return true;
            }
        }
    }
    
    return false;
}
