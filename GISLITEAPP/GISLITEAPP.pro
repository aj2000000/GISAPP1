QT += core gui widgets opengl network sql svg concurrent xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


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
    ui/layers \
    ui/theme \
    ui/users \
    ui/projects \
    ui/tracks \
    src \
    src/domain/models \
    src/domain/models/layers \
    src/domain/valueobjects \
    src/repositories/interfaces \
    src/repositories/sqlite \
    src/services \
    src/communication/udp \
    src/controllers \
    src/controllers/map \
    src/controllers/layers \
    src/ui_models \
    src/ui_models/layers \
    src/database \
    src/common



SOURCES += \
    main.cpp \
    src/MainApplication.cpp \
    src/database/DatabaseManager.cpp \
    src/domain/models/layers/LayerNode.cpp \
    src/domain/models/layers/LayerGroup.cpp \
    src/domain/models/layers/MapLayer.cpp \
    src/repositories/sqlite/SqliteLayerRepository.cpp \
    src/ui_models/layers/LayerTreeModel.cpp \
    src/controllers/map/MapController.cpp \
    src/controllers/layers/LayerController.cpp \
    ui/MainBaseUI.cpp \
    ui/HeaderBar.cpp \
    ui/LeftSidebar.cpp \
    ui/RightToolPanel.cpp \
    ui/ZoomControlsWidget.cpp \
    ui/TacticalStatusBar.cpp \
    ui/layers/LayerTreeView.cpp \
    ui/layers/LayerTreePanel.cpp \
    ui/map/MapWidget.cpp \
    ui/map/MapViewContainer.cpp \
    ui/theme/ThemeManager.cpp \
    ui/users/LoginPage.cpp \
    ui/users/RegisterPage.cpp \
    ui/users/AuthWindow.cpp

HEADERS += \
    src/MainApplication.h \
    src/database/DatabaseManager.h \
    src/domain/valueobjects/GeoCoordinate.h \
    src/domain/models/layers/LayerNode.h \
    src/domain/models/layers/LayerGroup.h \
    src/domain/models/layers/MapLayer.h \
    src/repositories/interfaces/ILayerRepository.h \
    src/repositories/sqlite/SqliteLayerRepository.h \
    src/ui_models/layers/LayerTreeModel.h \
    src/controllers/map/MapController.h \
    src/controllers/layers/LayerController.h \
    ui/MainBaseUI.h \
    ui/MainBasePage.h \
    ui/HeaderBar.h \
    ui/LeftSidebar.h \
    ui/RightToolPanel.h \
    ui/ZoomControlsWidget.h \
    ui/TacticalStatusBar.h \
    ui/layers/LayerTreeView.h \
    ui/layers/LayerTreePanel.h \
    ui/map/MapWidget.h \
    ui/map/MapViewContainer.h \
    ui/theme/ThemeManager.h \
    ui/users/LoginPage.h \
    ui/users/RegisterPage.h \
    ui/users/AuthWindow.h

FORMS +=

RESOURCES += \
    resources/resources.qrc
