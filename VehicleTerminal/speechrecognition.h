#ifndef SPEECHRECOGNITION_H
#define SPEECHRECOGNITION_H

#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDate>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QUrl>
#include <signal.h>
class SpeechRecognition : public QObject
{
    Q_OBJECT
  public:
    SpeechRecognition();

    // QThread interface
  private:
    void initAudioRecord();
    void initKeyRead();
    int readKeyValue();
    void initAsyncHandle();
  signals:
    void MessageFromAsrThread(QString str);
    void RecordFinished();
  private slots:
    void timer_timerout();
    static void getSpeechResult(QNetworkReply *reply);

  private:
    QAudioRecorder *m_audioRecorder = nullptr;
    QTimer *timer;
    int hasRecord = 0;
    QList<QVariant> devicesVar;
    QList<QVariant> codecsVar;
    QList<QVariant> containersVar;
    QList<QVariant> sampleRateVar;
    QList<QVariant> channelsVar;
    QList<QVariant> qualityVar;
    QList<QVariant> bitratesVar;

  public:
    int key_fd;
    void startSpeechRecognition();
    void startRecord();
    void stopRecord();
};

static SpeechRecognition *SR;
#endif // SPEECHRECOGNITION_H
