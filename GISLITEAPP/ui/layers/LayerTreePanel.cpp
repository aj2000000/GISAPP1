/**
 * @file LayerTreePanel.cpp
 * @brief Implementation of LayerTreePanel floating tactical layer manager panel.
 */

#include "LayerTreePanel.h"
#include "LayerTreeView.h"

#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

namespace GISApp::UI::Layers {

LayerTreePanel::LayerTreePanel(QWidget *parent)
    : QFrame(parent)
    , m_headerBar(nullptr)
    , m_titleLabel(nullptr)
    , m_countBadge(nullptr)
    , m_closeBtn(nullptr)
    , m_moveUpBtn(nullptr)
    , m_moveDownBtn(nullptr)
    , m_toggleBtn(nullptr)
    , m_addLayerBtn(nullptr)
    , m_filterEdit(nullptr)
    , m_treeView(nullptr)
    , m_isDragging(false)
{
    setObjectName("LayerTreePanel");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedSize(280, 420);

    setupUi();
    setupConnections();
}

void LayerTreePanel::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // 1. Draggable Header Bar
    m_headerBar = new QWidget(this);
    m_headerBar->setObjectName("LayerTreeHeader");
    auto *headerLayout = new QHBoxLayout(m_headerBar);
    headerLayout->setContentsMargins(4, 4, 4, 4);
    headerLayout->setSpacing(6);

    m_titleLabel = new QLabel("🗂️  GIS Layers", m_headerBar);
    m_titleLabel->setObjectName("LayerTreeTitle");

    m_countBadge = new QLabel("(0)", m_headerBar);
    m_countBadge->setObjectName("LayerTreeBadge");

    m_closeBtn = new QToolButton(m_headerBar);
    m_closeBtn->setObjectName("LayerTreeCloseButton");
    m_closeBtn->setText("✕");
    m_closeBtn->setToolTip("Close Layers Panel");
    m_closeBtn->setFixedSize(22, 22);

    headerLayout->addWidget(m_titleLabel);
    headerLayout->addWidget(m_countBadge);
    headerLayout->addStretch();
    headerLayout->addWidget(m_closeBtn);
    mainLayout->addWidget(m_headerBar);

    // 2. Action Toolbar: Move Up, Move Down, Toggle Visibility, Add Layer
    auto *toolbarWidget = new QWidget(this);
    toolbarWidget->setObjectName("LayerTreeToolbar");
    auto *toolbarLayout = new QHBoxLayout(toolbarWidget);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(4);

    m_panToBtn = new QToolButton(toolbarWidget);
    m_panToBtn->setObjectName("LayerTreeActionBtn");
    m_panToBtn->setText("🎯");
    m_panToBtn->setToolTip("Pan to Layer / Group");
    m_panToBtn->setFixedSize(30, 30);

    m_moveUpBtn = new QToolButton(toolbarWidget);
    m_moveUpBtn->setObjectName("LayerTreeActionBtn");
    m_moveUpBtn->setText("⬆️");
    m_moveUpBtn->setToolTip("Move Layer Up (Promote rendering z-order)");
    m_moveUpBtn->setFixedSize(30, 30);

    m_moveDownBtn = new QToolButton(toolbarWidget);
    m_moveDownBtn->setObjectName("LayerTreeActionBtn");
    m_moveDownBtn->setText("⬇️");
    m_moveDownBtn->setToolTip("Move Layer Down (Demote rendering z-order)");
    m_moveDownBtn->setFixedSize(30, 30);

    m_toggleBtn = new QToolButton(toolbarWidget);
    m_toggleBtn->setObjectName("LayerTreeActionBtn");
    m_toggleBtn->setText("👁️");
    m_toggleBtn->setToolTip("Toggle Layer Visibility");
    m_toggleBtn->setFixedSize(30, 30);

    m_addLayerBtn = new QToolButton(toolbarWidget);
    m_addLayerBtn->setObjectName("LayerTreeActionBtn");
    m_addLayerBtn->setText("➕");
    m_addLayerBtn->setToolTip("Add Layer / Dataset");
    m_addLayerBtn->setFixedSize(30, 30);

    toolbarLayout->addWidget(m_panToBtn);
    toolbarLayout->addWidget(m_moveUpBtn);
    toolbarLayout->addWidget(m_moveDownBtn);
    toolbarLayout->addWidget(m_toggleBtn);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_addLayerBtn);
    mainLayout->addWidget(toolbarWidget);

    // 3. Search / Filter Edit
    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setObjectName("LayerFilterEdit");
    m_filterEdit->setPlaceholderText("🔍 Filter layers...");
    m_filterEdit->setClearButtonEnabled(true);
    m_filterEdit->setFixedHeight(28);
    mainLayout->addWidget(m_filterEdit);

    // 4. Custom Tree View
    m_treeView = new LayerTreeView(this);
    mainLayout->addWidget(m_treeView, 1);
}

void LayerTreePanel::setupConnections()
{
    connect(m_closeBtn, &QToolButton::clicked, this, [this]() {
        hide();
        emit closeRequested();
    });

    connect(m_panToBtn, &QToolButton::clicked, this, [this]() {
        emit panToRequested(selectedIndex());
    });
    connect(m_moveUpBtn, &QToolButton::clicked, this, &LayerTreePanel::moveUpRequested);
    connect(m_moveDownBtn, &QToolButton::clicked, this, &LayerTreePanel::moveDownRequested);
    connect(m_toggleBtn, &QToolButton::clicked, this, &LayerTreePanel::toggleVisibilityRequested);
    connect(m_addLayerBtn, &QToolButton::clicked, this, &LayerTreePanel::addLayerRequested);

    // Live search filter connection
    connect(m_filterEdit, &QLineEdit::textChanged, this, [this](const QString &query) {
        if (!m_treeView || !m_treeView->model()) return;
        auto filterRecursive = [this](auto &self, const QModelIndex &parent, const QString &text) -> bool {
            bool hasVisibleDescendant = false;
            int rows = m_treeView->model()->rowCount(parent);
            for (int r = 0; r < rows; ++r) {
                QModelIndex idx = m_treeView->model()->index(r, 0, parent);
                QString name = m_treeView->model()->data(idx, Qt::DisplayRole).toString();
                bool matches = text.isEmpty() || name.contains(text, Qt::CaseInsensitive);
                bool childMatch = self(self, idx, text);
                bool shouldShow = matches || childMatch;
                m_treeView->setRowHidden(r, parent, !shouldShow);
                if (shouldShow) {
                    hasVisibleDescendant = true;
                    if (!text.isEmpty()) {
                        m_treeView->expand(idx);
                    }
                }
            }
            return hasVisibleDescendant;
        };
        filterRecursive(filterRecursive, QModelIndex(), query.trimmed());
    });

    if (m_treeView) {
        connect(m_treeView, &LayerTreeView::panToTriggered, this, &LayerTreePanel::panToRequested);
        connect(m_treeView, &LayerTreeView::moveUpTriggered, this, &LayerTreePanel::moveUpRequested);
        connect(m_treeView, &LayerTreeView::moveDownTriggered, this, &LayerTreePanel::moveDownRequested);
        connect(m_treeView, &LayerTreeView::toggleVisibilityTriggered, this, &LayerTreePanel::toggleVisibilityRequested);
    }
}

void LayerTreePanel::setModel(QAbstractItemModel *model)
{
    if (m_treeView) {
        m_treeView->setModel(model);
        m_treeView->expandAll();
    }
}

QModelIndex LayerTreePanel::selectedIndex() const
{
    if (!m_treeView || !m_treeView->selectionModel()) {
        return QModelIndex();
    }
    QModelIndexList selected = m_treeView->selectionModel()->selectedRows();
    return selected.isEmpty() ? QModelIndex() : selected.first();
}

void LayerTreePanel::updateLayerCount(int count)
{
    if (m_countBadge) {
        m_countBadge->setText(QString("(%1)").arg(count));
    }
}

void LayerTreePanel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_headerBar && m_headerBar->geometry().contains(event->pos())) {
        m_isDragging = true;
        m_dragStartPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QFrame::mousePressEvent(event);
}

void LayerTreePanel::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
        if (parentWidget()) {
            QPoint newPos = event->globalPosition().toPoint() - m_dragStartPosition;
            // Clamp within parent bounds
            int maxX = parentWidget()->width() - width();
            int maxY = parentWidget()->height() - height();
            newPos.setX(std::clamp(newPos.x(), 0, std::max(0, maxX)));
            newPos.setY(std::clamp(newPos.y(), 0, std::max(0, maxY)));
            move(newPos);
        } else {
            move(event->globalPosition().toPoint() - m_dragStartPosition);
        }
        event->accept();
        return;
    }
    QFrame::mouseMoveEvent(event);
}

void LayerTreePanel::mouseReleaseEvent(QMouseEvent *event)
{
    m_isDragging = false;
    QFrame::mouseReleaseEvent(event);
}

} // namespace GISApp::UI::Layers
