#include "rson.h"
#include "ui_rson.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QSaveFile>
#include <QSerialPortInfo>
#include <QStringBuilder>

#include <QDebug>

RSON::RSON(QWidget *parent, const QString radioFilename)
    : QMainWindow(parent)
    , ui(new Ui::RSON)
    , loadedFilename(radioFilename)

{
    ui->setupUi(this);
    statusBar()->showMessage("Loaded JSON file: NONE");
    ui->tabWidget->setCurrentIndex(0);
    loadSerialPortList();
    // qDebug()<<"loadedFilename:"<<loadedFilename;
    if(!loadedFilename.isEmpty()) {
        loadDataFromFile(loadedFilename);
    }
    // hide the Memory Channel radio button for now
    ui->memChannelRadioButton->setVisible(false);
}

RSON::~RSON()
{
    delete ui;
}

void RSON::loadSerialPortList() {
    ui->radioAddressComboBox->clear();
    QStringList addrList;
    QList<QSerialPortInfo> addrs = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo spi, addrs) {
        addrList<<spi.portName();
    }
    ui->radioAddressComboBox->addItems(addrList);
}

void RSON::on_pttMethodCATRadioButton_toggled(bool checked)
{
    if(checked) {
        // QStringList addrList;
        // QList<QSerialPortInfo> addrs = QSerialPortInfo::availablePorts();
        // foreach (const QSerialPortInfo spi, addrs) {
        //     addrList<<spi.portName();
        // }
        // ui->radioAddressComboBox->addItems(addrList);
        loadSerialPortList();
        ui->radioAddressComboBox->showPopup();
        ui->tcpPortNumberGroupBox->setEnabled(false);
        ui->serialDetailsGroupBox->setEnabled(true);
    }
    else {
        ui->radioAddressComboBox->clear();
        ui->radioAddressComboBox->setFocus();
        ui->tcpPortNumberGroupBox->setEnabled(true);
        ui->serialDetailsGroupBox->setEnabled(false);
    }
}

void RSON::on_refreshSerialButton_clicked()
{
    // qDebug()<<"refresh the list based on connection type";
    ui->radioAddressComboBox->clear();
    if(ui->pttMethodCATRadioButton->isChecked()) {
        QStringList addrList;
        QList<QSerialPortInfo> addrs = QSerialPortInfo::availablePorts();
        foreach (const QSerialPortInfo spi, addrs) {
            addrList<<spi.portName();
        }
        ui->radioAddressComboBox->addItems(addrList);
        ui->radioAddressComboBox->showPopup();
    }
}

void RSON::on_action_Save_to_JSON_triggered() {
    QJsonDocument jd;
    QJsonObject jo;
    jo.insert("radioName", QJsonValue(ui->radioNameLineEdit->text().trimmed()));
    // QString tmp = ui->controlTypeCIVRadioButton->isChecked()?"BINARY":"ASCII";
    // jo.insert("controlType", QJsonValue(tmp));
    QString tmp = ui->pttMethodTCPRadioButton->isChecked()?"TCP":"SERIAL";
    jo.insert("connectionMethod", QJsonValue(tmp));
    jo.insert("radioAddress", QJsonValue(ui->radioAddressComboBox->currentText().trimmed().toUpper()));
    jo.insert("tcpPortNumber", QJsonValue(ui->tcpPortSpinBox->value()));
    jo.insert("portTimeout", QJsonValue(ui->portTimeoutSpinBox->value()));
    jo.insert("commandTimeout", QJsonValue(ui->commandTimeoutSpinBox->value()));
    jo.insert("initialSetup", QJsonValue(ui->setupLineEdit->text().trimmed().toUpper()));
    jo.insert("pttOn", QJsonValue(ui->pttOnLineEdit->text().trimmed().toUpper()));
    jo.insert("pttOff", QJsonValue(ui->pttOffLineEdit->text().trimmed().toUpper()));
    jo.insert("pttData", QJsonValue(ui->pttDataLineEdit->text().trimmed().toUpper()));
    jo.insert("autoTune", QJsonValue(ui->autoTuneLineEdit->text().trimmed().toUpper()));
    jo.insert("txTail", QJsonValue(ui->txTailSpinBox->value()));
    // the serial details
    jo.insert("serialBaudRate",QJsonValue(ui->serialBaudComboBox->currentText().toInt()));
    jo.insert("serialParams",QJsonValue(ui->serialParamsComboBox->currentText().toUpper()));
    tmp = ui->serialFlowControlComboBox->currentText().trimmed();
    jo.insert("serialFlowControl",QJsonValue(tmp.isEmpty()?"No Flow Control" : tmp));
    // the list of standard modes "modeList"
    QJsonObject jomodes;
    jomodes.insert("USB", QJsonValue(ui->usbModeLineEdit->text().trimmed().toUpper()));
    jomodes.insert("LSB", QJsonValue(ui->lsbModeLineEdit->text().trimmed().toUpper()));
    jomodes.insert("CW", QJsonValue(ui->cwModeLineEdit->text().trimmed().toUpper()));
    jomodes.insert("AM", QJsonValue(ui->amModeLineEdit->text().trimmed().toUpper()));
    jomodes.insert("RTTY", QJsonValue(ui->rttyLineEdit->text().trimmed().toUpper()));
    jomodes.insert("DATA1", QJsonValue(ui->data1LineEdit->text().trimmed().toUpper()));
    jomodes.insert("DATA2", QJsonValue(ui->data2LineEdit->text().trimmed().toUpper()));
    jomodes.insert("DATA3", QJsonValue(ui->data3LineEdit->text().trimmed().toUpper()));
    // add the mode list to the main object
    jo.insert("modeList", jomodes);
    // TODO: now the user modes tab modes
    // END USER MODES LIST
    // frequency control values
    QJsonObject jofreq;
    tmp = ui->freqDigitTypeCIVRadioButton->isChecked() ? "CI-V" : ui->freqDigitTypeCATRadioButton->isChecked() ? "CAT" : "BCD";
    qDebug()<<"freq order:"<<tmp;
    jofreq.insert("order", QJsonValue(tmp));
    int itmp = ui->freqDigitsSpinBox->value();
    if((itmp % 2) > 0) {
        bool keep = QMessageBox::question(this, "Odd Number of Digits?", "You have specified an odd number of frequency digits.\n\nIs this correct?") == QMessageBox::Yes;
        if (keep) { jofreq.insert("numDigits", QJsonValue(itmp)); }
        else { ui->freqDigitsSpinBox->setFocus(); return; };
    }

    jofreq.insert("prefixA", QJsonValue(ui->freqPrefixALineEdit->text().trimmed().toUpper()));
    tmp = ui->freqPrefixBLineEdit->text().trimmed().toUpper();
    // set the B value to the A value if empty
    if(tmp.isEmpty()) { tmp = ui->freqPrefixALineEdit->text().trimmed().toUpper(); }
    jofreq.insert("prefixB", QJsonValue(tmp));
    jofreq.insert("numDigits", QJsonValue(ui->freqDigitsSpinBox->value()).toInt(8));
    tmp = ui->freqDigitTypeCATRadioButton->isChecked()?"CAT":ui->freqDigitTypeCIVRadioButton->isChecked()?"CI-V":"BCD";
    qDebug()<<"save to JSON order:"<<tmp;
    jofreq.insert("order", QJsonValue(tmp));
    jofreq.insert("suffix", ui->freqSuffixLineEdit->text().trimmed().toUpper());
    jo.insert("frequencyControl", jofreq);
    // build and add query command list
    QJsonObject joquery;
    joquery.insert("vfoAFreq", QJsonValue(ui->vfoAQueryLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfoAFreqResponse", QJsonValue(ui->vfoAFreqResponseLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfoBFreq", QJsonValue(ui->vfoBQueryLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfoBFreqResponse", QJsonValue(ui->vfoBFreqResponseLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfo", QJsonValue(ui->vfoActiveQueryLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfoAResponse", QJsonValue(ui->vfoAQueryResponseLineEdit->text().trimmed().toUpper()));
    joquery.insert("vfoBResponse", QJsonValue(ui->vfoBQueryResponseLineEdit->text().trimmed().toUpper()));
    joquery.insert("mode", QJsonValue(ui->currModeQueryLineEdit->text().trimmed().toUpper()));
    joquery.insert("modeResponsePrefix", QJsonValue(ui->currModeResponseLineEdit->text().trimmed().toUpper()));
    // add query commands to the root object
    jo.insert("queryCommands", joquery);
    // add the root object to the document
    jd.setObject(jo);
    // convert to JSON text
    QByteArray json = jd.toJson();
    qDebug()<<"JSON OUT:"<<json;
    QString fname = QFileDialog::getSaveFileName(this, "Save RSON File", "./RadioFiles", "*.json");
    if(fname.isEmpty()) { return; }
    QSaveFile out = QSaveFile(fname);
    if(out.open(QFile::WriteOnly)) {
        out.write(json);
    }
    out.commit();
    loadDataFromFile(fname);
}

void RSON::on_action_Import_JSON_triggered() {
    QString fn = QFileDialog::getOpenFileName(this, "Load an RSON File (JSON)", "./RadioFiles", "*.json");
    if(fn.isEmpty()) { return; }
    loadDataFromFile(fn);
}

void RSON::loadDataFromFile(const QString fn) {
    QByteArray json;
    QFile in = QFile(fn);
    if(in.open(QFile::ReadOnly)) {
        json = in.readAll();
        in.close();
    }
    loadedFilename = fn;
    statusBar()->showMessage("File Loaded: " % fn.split("/").last());
    QJsonDocument jd = QJsonDocument::fromJson(json);
    if(jd.isObject()) {
        QJsonObject jo = jd.object();
        // now we can walk the object to load the UI
        // radio tab
        ui->radioNameLineEdit->setText(jo.value("radioName").toString());
//        ui->controlTypeCIVRadioButton->setChecked(jo.value("controlType").toString() == "BINARY");
        ui->pttMethodCATRadioButton->setChecked(jo.value("pttMethod").toString() == "CAT");
        ui->autoTuneLineEdit->setText(jo.value("autoTune").toString());
        ui->radioAddressComboBox->setCurrentText(jo.value("radioAddress").toString());
        int itmp = jo.value("serialBaudRate").toInt();
        ui->serialBaudComboBox->setCurrentText(QString::number(itmp));
        ui->serialParamsComboBox->setCurrentText(jo.value("serialParams").toString());
        ui->serialFlowControlComboBox->setCurrentText(jo.value("serialFlowControl").toString());
        ui->tcpPortSpinBox->setValue(jo.value("tcpPortNumber").toInt());
        ui->portTimeoutSpinBox->setValue(jo.value("portTimeout").toInt());
        ui->commandTimeoutSpinBox->setValue(jo.value("commandTimeout").toInt());
        // setup tab
        QString stmp = jo.value("initialSetup").toString().trimmed();
        ui->setupLineEdit->setText(stmp);
        ui->pttOnLineEdit->setText(jo.value("pttOn").toString());
        ui->pttOffLineEdit->setText(jo.value("pttOff").toString());
        stmp = jo.value("pttData").toString().trimmed();
        ui->pttDataLineEdit->setText(stmp);
        itmp = jo.value("txTail").toInt();
        ui->txTailSpinBox->setValue(itmp > -1 ? itmp : 20);
        // modes tab
        QJsonObject jtmp = jo.value("modeList").toObject();
        QStringList keys = jtmp.keys();
        foreach(const QString key, keys) {
            if(key == "USB") {
                ui->usbModeLineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "LSB") {
                ui->lsbModeLineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "CW") {
                ui->cwModeLineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "AM") {
                ui->amModeLineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "RTTY") {
                ui->rttyLineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "DATA1") {
                ui->data1LineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "DATA2") {
                ui->data2LineEdit->setText(jtmp.value(key).toString());
            }
            if(key == "DATA3") {
                ui->data3LineEdit->setText(jtmp.value(key).toString());
            }
            // OTHER user created modes will not be displayed
        }
        jtmp = jo.value("frequencyControl").toObject();
        ui->freqDigitsSpinBox->setValue(jtmp.value("numDigits").toInt(8));
        ui->freqPrefixALineEdit->setText(jtmp.value("prefixA").toString());
        ui->freqPrefixBLineEdit->setText(jtmp.value("prefixB").toString());
        ui->freqSuffixLineEdit->setText(jtmp.value("suffix").toString());
        qDebug()<<"loadDataFromFile order:"<<jtmp.value("order").toString();
        if(jtmp.value("order").toString() == "CI-V") {
            ui->freqDigitTypeCIVRadioButton->setChecked(true);
        }
        else if(jtmp.value("order").toString() == "CAT") {
            ui->freqDigitTypeCATRadioButton->setChecked(true);
        }
        else {
            ui->freqDigitTypeBCDRadioButton->setChecked(true);
        }
        // load the query command list
        jtmp = jo.value("queryCommands").toObject();
        if(!jtmp.isEmpty()) {
            // next two are for frequency queries
            ui->vfoAQueryLineEdit->setText(jtmp.value("vfoAFreq").toString());
            ui->vfoBQueryLineEdit->setText(jtmp.value("vfoBFreq").toString());
            // two possible responses from radio for freq queries
            ui->vfoAFreqResponseLineEdit->setText(jtmp.value("vfoAFreqResponse").toString());
            ui->vfoBFreqResponseLineEdit->setText(jtmp.value("vfoBFreqResponse").toString());
            // which vfo is active?
            ui->vfoActiveQueryLineEdit->setText(jtmp.value("vfo").toString());
            // two possible responses from radio
            ui->vfoAQueryResponseLineEdit->setText(jtmp.value("vfoAResponse").toString());
            ui->vfoBQueryResponseLineEdit->setText(jtmp.value("vfoBResponse").toString());
            // query the current mode, which is matched to the modeList value to
            // reply with the mode name.
            ui->currModeQueryLineEdit->setText(jtmp.value("mode").toString());
            ui->currModeResponseLineEdit->setText(jtmp.value("modeResponsePrefix").toString());
            // ui->pttStateQueryLineEdit->setText(jtmp.value("ptt").toString());
            // ui->splitStateQueryLineEdit->setText(jtmp.value("split").toString());
        }
    }
}

void RSON::on_action_Report_Format_triggered()
{
    // throw up a QMessageBox::information with a readable version
    // of the loaded configuration.
    // If there is no known filename, just tell them to load a file first.
    if(loadedFilename.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Configuration Not Loaded", "Load a configuration JSON file first!!");
        return;
    }
    QByteArray json;
    QFile jsonFile = QFile(loadedFilename);
    if(jsonFile.open(QFile::ReadOnly)) {
        json = jsonFile.readAll().trimmed();
        jsonFile.close();
    }
    QJsonDocument jd = QJsonDocument::fromJson(json);
    QString out;
    if(jd.isObject()) {
        QJsonObject jo = jd.object();
        out.append("--- RSON Report for " % jo.value("radioName").toString() % " ---" % CRLF);
        out.append("Control Type: " % jo.value("controlType").toString() % CRLF);
        out.append("PTT Method: " % jo.value("pttMethod").toString() % CRLF);
        out.append("AutoTune:" % jo.value("autoTune").toString() % CRLF);
        out.append("Radio Address: " % jo.value("radioAddress").toString() % CRLF);
        out.append("--- Serial Details ---" % CRLF);
        out.append("Baud Rate: " % QString::number(jo.value("serialBaudRate").toInt()) % CRLF);
        out.append("Serial Params: " % jo.value("serialParams").toString() % CRLF);
        out.append("Flow Control: " % jo.value("serialFlowControl").toString() % CRLF);
        int tmp = jo.value("tcpPortNumber").toInt();
        out.append("TCP Port Number: " % (tmp>0?QString::number(tmp):"N/A") % CRLF);
        out.append("Port Timeout: " % jo.value("portTimeout").toString() % CRLF);
        out.append("Command Timeout: " % jo.value("commandTimeout").toString() % CRLF);
        out.append(CRLF % "Initial Setup: " % jo.value("initialSetup").toString() % CRLF);
        out.append("PTT on: " % jo.value("pttOn").toString() % CRLF);
        out.append("PTT off: " % jo.value("pttOff").toString() % CRLF);
        out.append("PTT data: " % jo.value("pttData").toString() % CRLF);
        out.append("Tx Tail: " % jo.value("txTail").toString() % CRLF);
        out.append("--- Mode List ---" % CRLF);
        QJsonObject jotmp = jo.value("modeList").toObject();
        QStringList keys = jotmp.keys();
        foreach(const QString key, keys) {
            out.append(key % ": " % jotmp.value(key).toString() % CRLF);
        }
        out.append("--- Query Commands ---" % CRLF);
        jotmp = jo.value("queryCommands").toObject();
        qDebug()<<"Query Cmds JSON Object"<<jotmp;
        if(!jotmp.isEmpty()) {
            out.append("VFO A Freq Query: ").append(jotmp.value("vfoAFreq").toString());
            out.append("VFO A Freq Response: ").append(jotmp.value("vfoAFreqResponse").toString());
            out.append("VFO B Freq Query: ").append(jotmp.value("vfoBFreq").toString());
            out.append("VFO B Freq Response: ").append(jotmp.value("vfoBFreqResponse").toString());
            out.append("VFO A/B Query: ").append(jotmp.value("vfo").toString());
            out.append("Mode Query: ").append(jotmp.value("mode").toString());
            out.append("Mode Response: ").append(jotmp.value("modeResponsePrefix").toString());
        }
        // qDebug().noquote()<<"RSON Report:"<<out;
        QMessageBox::information(this, "RSON Report for " % loadedFilename, out);
    }
    else {
        QMessageBox::warning(this, "RSON Parsing% Error", "The JSON file is not parse-able.");
    }

}

void RSON::on_action_About_RSON_triggered()
{
    QMessageBox::information(this, "About RSON", "Burning down old radio control! :) -- FESTIVUS!\r\n\r\nRSON helps the user to create \
a new ARCON file, which is in JSON format, that holds all the necessary commands for most radio control activities.\r\n\r\n \
Copyright 2026 GrizzWorks, LLC -- All Rights Reserved");
}

void RSON::on_actionE_xit_triggered()
{
    close();
}

void RSON::on_controlTypeCIVRadioButton_toggled(bool checked)
{
    if(checked) {
        ui->freqDigitTypeCIVRadioButton->setChecked(true);
    }
}

