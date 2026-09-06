#ifndef ISAMPLEENTITYREPOSITORY_H
#define ISAMPLEENTITYREPOSITORY_H




#include <QObject>
#include <QVector>
#include <optional>
#include "sampleentity.h"

namespace GISApp::Repositories {


class ISampleEntityRepository : public QObject
{
    Q_OBJECT

public:
    explicit ISampleEntityRepository(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ISampleEntityRepository() override = default;


    virtual void upsertSampleEntity(const Domain::SampleEntities::SampleEntity &sampleEntity) = 0;


    virtual void upsertSampleEntities(const QVector<Domain::SampleEntities::SampleEntity> &sampleEntity) = 0;


    [[nodiscard]] virtual QVector<Domain::SampleEntities::SampleEntity> getAllSampleEntities() const = 0;


    [[nodiscard]] virtual std::optional<Domain::SampleEntities::SampleEntity> getSampleEntityById(int id) const = 0;


    virtual bool removeSampleEntity(int Id) = 0;


    virtual void clearSampleEntity() = 0;


    [[nodiscard]] virtual int count() const = 0;

signals:

    void sampleEntitiesUpdated();


    void sampleEntityUpserted(int SampleEntityId);

    void sampleEntityRemoved(int SampleEntityId);
};

} // namespace GISApp::Repositories





#endif // ISAMPLEENTITYREPOSITORY_H
