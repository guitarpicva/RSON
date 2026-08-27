#ifndef RSON_H
#define RSON_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class RSON;
}
QT_END_NAMESPACE

class RSON : public QMainWindow
{
    Q_OBJECT

public:
    explicit RSON(QWidget *parent = nullptr, const QString radioFilename = "");
    ~RSON() override;

private slots:

    void on_pttMethodCATRadioButton_toggled(bool checked);
    void on_refreshSerialButton_clicked();
    void loadSerialPortList();
    void on_action_Save_to_JSON_triggered();
    void on_action_Import_JSON_triggered();
    void on_action_Report_Format_triggered();
    void on_action_About_RSON_triggered();
    void on_actionE_xit_triggered();

private:
    Ui::RSON *ui;
    QString loadedFilename;
    const QString CRLF = "\r\n";
    void loadDataFromFile(const QString fn);
};
#endif // RSON_H
