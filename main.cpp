#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QSharedMemory>
#include "SystemTray.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  // Ngăn chặn mở nhiều cửa sổ ứng dụng (Single instance protection)
  QSharedMemory sharedMemory("LR_02_SingleInstanceSharedMemoryKey");
  if (!sharedMemory.create(1)) {
    return 0; // Đã có phiên bản chạy trước đó, thoát ngay
  }

  QQuickStyle::setStyle("Basic");

  QQmlApplicationEngine engine;

  // Khởi tạo giao diện
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
      []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);
  engine.loadFromModule("LR_02/qml", "Home");

  return QGuiApplication::exec();
}
