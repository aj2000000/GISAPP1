QT += core gui widgets opengl network sql svg concurrent xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= flat


# Build Output Directories
DESTDIR     = $$PWD/build/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR     = $$PWD/build/moc
UI_DIR      = $$PWD/build/ui


MAPLIBRE_INSTALL_DIR = $$(MAPLIBRE_INSTALL_DIR)
isEmpty(MAPLIBRE_INSTALL_DIR) {
    MAPLIBRE_INSTALL_DIR = $$(HOME)/maplibre-install
}

INCLUDEPATH += $$MAPLIBRE_INSTALL_DIR/include \
               $$MAPLIBRE_INSTALL_DIR/include/QMapLibre \
               $$MAPLIBRE_INSTALL_DIR/include/QMapLibreWidgets \
               /usr/include/gdal

LIBS += -L$$MAPLIBRE_INSTALL_DIR/lib -lQMapLibre -lQMapLibreWidgets -lgdal -lcrypto

QMAKE_LFLAGS += -Wl,-rpath,$$MAPLIBRE_INSTALL_DIR/lib

TARGET = GISLITEAPP
TEMPLATE = app

INCLUDEPATH += \
    ui \
    ui/map \
    ui/map/renderers \
    ui/layers \
    ui/theme \
    ui/users \
    ui/projects \
    ui/tracks \
    src \
    src/domain/models \
    src/domain/models/layers \
    src/domain/models/tracks \
    src/domain/valueobjects \
    src/repositories/interfaces \
    src/repositories/sqlite \
    src/repositories/tracks \
    src/services \
    src/services/tracks \
    src/communication/udp \
    src/communication/udp/transport \
    src/communication/udp/protocol \
    src/communication/udp/security \
    src/communication/udp/config \
    src/communication/udp/handlers \
    src/controllers \
    src/controllers/map \
    src/controllers/layers \
    src/controllers/tracks \
    src/ui_models \
    src/ui_models/layers \
    src/ui_models/tracks \
    src/core \
    src/core/interfaces \
    src/core/wrappers \
    src/database \
    src/common

SOURCES += \
    main.cpp \
    src/MainApplication.cpp \
    src/core/wrappers/BaseTablePanelDialog.cpp \
    src/core/wrappers/BaseMapFeatureRenderer.cpp \
    src/database/DatabaseManager.cpp \
    src/domain/models/layers/LayerNode.cpp \
    src/domain/models/layers/LayerGroup.cpp \
    src/domain/models/layers/MapLayer.cpp \
    src/domain/models/tracks/TacticalTrack.cpp \
    src/repositories/sqlite/SqliteLayerRepository.cpp \
    src/repositories/tracks/TrackRepository.cpp \
    src/ui_models/layers/LayerTreeModel.cpp \
    src/ui_models/tracks/TrackTableModel.cpp \
    src/controllers/map/MapController.cpp \
    src/controllers/layers/LayerController.cpp \
    src/controllers/tracks/TrackController.cpp \
    src/services/tracks/TacticalTrackService.cpp \
    src/communication/udp/UdpServiceMediator.cpp \
    src/communication/udp/transport/UdpReceiver.cpp \
    src/communication/udp/transport/UdpSender.cpp \
    src/communication/udp/transport/UdpPacketProcessor.cpp \
    src/communication/udp/security/Aes256Cipher.cpp \
    src/communication/udp/config/UdpDataStore.cpp \
    src/communication/udp/handlers/UdpMessageDispatcher.cpp \
    src/communication/udp/handlers/UdpTrackMessageHandler.cpp \
    ui/MainBaseUI.cpp \
    ui/HeaderBar.cpp \
    ui/LeftSidebar.cpp \
    ui/RightToolPanel.cpp \
    ui/ZoomControlsWidget.cpp \
    ui/TacticalStatusBar.cpp \
    ui/layers/LayerTreeView.cpp \
    ui/layers/LayerTreePanel.cpp \
    ui/tracks/TrackTablePanelDialog.cpp \
    ui/map/MapWidget.cpp \
    ui/map/MapViewContainer.cpp \
    ui/map/renderers/TrackMapRenderer.cpp \
    ui/theme/ThemeManager.cpp \
    ui/users/LoginPage.cpp \
    ui/users/RegisterPage.cpp \
    ui/users/AuthWindow.cpp \
    ui/tracks/TrackDetailDialog.cpp \
    src/common/fieldkeyvaluemapper.cpp

HEADERS += \
    src/MainApplication.h \
    src/common/fieldkeyvaluemapper.h \
    src/core/interfaces/ITablePanelDialog.h \
    src/core/interfaces/IMapFeature.h \
    src/core/interfaces/IMapFeatureRenderer.h \
    src/core/interfaces/IContextMenuContributor.h \
    ui/tracks/TrackDetailDialog.h \
    src/core/wrappers/BaseTablePanelDialog.h \
    src/core/wrappers/BaseMapFeatureRenderer.h \
    src/database/DatabaseManager.h \
    src/domain/valueobjects/GeoCoordinate.h \
    src/domain/models/layers/LayerNode.h \
    src/domain/models/layers/LayerGroup.h \
    src/domain/models/layers/MapLayer.h \
    src/domain/models/tracks/TacticalTrack.h \
    src/repositories/interfaces/ILayerRepository.h \
    src/repositories/interfaces/ITrackRepository.h \
    src/repositories/sqlite/SqliteLayerRepository.h \
    src/repositories/tracks/TrackRepository.h \
    src/ui_models/layers/LayerTreeModel.h \
    src/ui_models/tracks/TrackTableModel.h \
    src/controllers/map/MapController.h \
    src/controllers/layers/LayerController.h \
    src/controllers/tracks/TrackController.h \
    src/services/tracks/TacticalTrackService.h \
    src/communication/udp/UdpServiceMediator.h \
    src/communication/udp/transport/UdpReceiver.h \
    src/communication/udp/transport/UdpSender.h \
    src/communication/udp/transport/UdpPacketProcessor.h \
    src/communication/udp/security/Aes256Cipher.h \
    src/communication/udp/config/UdpDataStore.h \
    src/communication/udp/protocol/IrsTypes.h \
    src/communication/udp/protocol/MessageIds.h \
    src/communication/udp/protocol/WireStructures.h \
    src/communication/udp/handlers/IUdpMessageHandler.h \
    src/communication/udp/handlers/UdpMessageDispatcher.h \
    src/communication/udp/handlers/UdpTrackMessageHandler.h \
    ui/MainBaseUI.h \
    ui/MainBasePage.h \
    ui/HeaderBar.h \
    ui/LeftSidebar.h \
    ui/RightToolPanel.h \
    ui/ZoomControlsWidget.h \
    ui/TacticalStatusBar.h \
    ui/layers/LayerTreeView.h \
    ui/layers/LayerTreePanel.h \
    ui/tracks/TrackTablePanelDialog.h \
    ui/map/MapWidget.h \
    ui/map/MapViewContainer.h \
    ui/map/renderers/TrackMapRenderer.h \
    ui/map/renderers/TrackMapFeatureAdapter.h \
    ui/theme/ThemeManager.h \
    ui/users/LoginPage.h \
    ui/users/RegisterPage.h \
    ui/users/AuthWindow.h

FORMS +=

RESOURCES += \
    resources/resources.qrc
