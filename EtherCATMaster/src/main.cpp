#include "MainWindow.h"
#include "EthercatTypes.h"

#include <QApplication>
#include <QIcon>

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // Keep taskbar pinning/grouping stable across runs.
    using SetAppIdFn = HRESULT(WINAPI*)(PCWSTR);
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");
    if (shell32 != nullptr)
    {
        auto set_app_id = reinterpret_cast<SetAppIdFn>(
            GetProcAddress(shell32, "SetCurrentProcessExplicitAppUserModelID"));
        if (set_app_id != nullptr)
        {
            set_app_id(L"ClearCore.EtherCATMasterQt");
        }
        FreeLibrary(shell32);
    }
#endif
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app_icon.svg")));
    qRegisterMetaType<MasterSnapshot>("MasterSnapshot");

    MainWindow window;
    window.show();

    return app.exec();
}
