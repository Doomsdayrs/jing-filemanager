#include "android.h"
#include <QException>
#include <QColor>
#include <QDebug>

class InterfaceConnFailedException : public QException
{
public:
    void raise() const { throw *this; }
    InterfaceConnFailedException *clone() const { return new InterfaceConnFailedException(*this); }
};

Android::Android(QObject *parent) : QObject(parent)
{

}

void Android::statusbarColor(const QString &bg, const bool &light)
{
    QtAndroid::runOnAndroidThread([=]() {
        QAndroidJniObject window = QtAndroid::androidActivity().callObjectMethod("getWindow", "()Landroid/view/Window;");
        window.callMethod<void>("addFlags", "(I)V", 0x80000000);
        window.callMethod<void>("clearFlags", "(I)V", 0x04000000);
        window.callMethod<void>("setStatusBarColor", "(I)V", QColor(bg).rgba());

        if(light)
        {
            QAndroidJniObject decorView = window.callObjectMethod("getDecorView", "()Landroid/view/View;");
            decorView.callMethod<void>("setSystemUiVisibility", "(I)V", 0x00002000);
        }

    });

}

void Android::shareDialog(const QString &url)
{
    qDebug()<< "trying to share dialog";
    QAndroidJniEnvironment _env;
    QAndroidJniObject activity = QAndroidJniObject::callStaticObjectMethod("org/qtproject/qt5/android/QtNative", "activity", "()Landroid/app/Activity;");   //activity is valid
    if (_env->ExceptionCheck()) {
        _env->ExceptionClear();
        throw InterfaceConnFailedException();
    }
    if ( activity.isValid() )
    {
        QAndroidJniObject::callStaticMethod<void>("com/example/android/tools/SendIntent",
                                                  "sendPic",
                                                  "(Landroid/app/Activity;Ljava/lang/String;)V",
                                                  activity.object<jobject>(),
                                                  QAndroidJniObject::fromString(url).object<jstring>());
        if (_env->ExceptionCheck()) {
            _env->ExceptionClear();
            throw InterfaceConnFailedException();
        }
    }else
        throw InterfaceConnFailedException();
}

QString Android::homePath()
{

    QAndroidJniObject mediaDir = QAndroidJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
    QAndroidJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );

    return mediaPath.toString();
}


QStringList Android::defaultPaths()
{
    QStringList paths;

    paths.append(PATHS::HomePath);
    paths.append(PATHS::DocumentsPath);
    paths.append(PATHS::MusicPath);
    paths.append(PATHS::VideosPath);
    paths.append(PATHS::PicturesPath);
    paths.append(PATHS::DownloadsPath);

    return paths;
}


