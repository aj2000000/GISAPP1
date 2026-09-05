/**
 * @file TrackMapRenderer.cpp
 * @brief Implementation of TrackMapRenderer specializing BaseMapFeatureRenderer.
 * @author BrahmaxisGIS Development Team
 * @date 2026
 */

#include "TrackMapRenderer.h"

namespace GISApp::UI::Renderers {

TrackMapRenderer::TrackMapRenderer(GISApp::UI::MapWidget *mapWidget, QObject *parent)
    : BaseMapFeatureRenderer(QStringLiteral("source_tactical_tracks"),
                             QStringLiteral("tactical_tracks"),
                             mapWidget,
                             parent)
{
}

} // namespace GISApp::UI::Renderers
