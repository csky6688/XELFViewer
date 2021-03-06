// Copyright (c) 2019-2020 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include "guimainwindow.h"
#include "ui_guimainwindow.h"

GuiMainWindow::GuiMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GuiMainWindow)
{
    ui->setupUi(this);

    pFile=nullptr;
    pWidget=nullptr;

    ui->stackedWidget->setCurrentIndex(0);

    setWindowTitle(QString("%1 v%2").arg(X_APPLICATIONNAME).arg(X_APPLICATIONVERSION));

    setAcceptDrops(true);

    xOptions.setName("xelfviewer");

    QList<XOptions::ID> listIDs;

    listIDs.append(XOptions::ID_STAYONTOP);
    listIDs.append(XOptions::ID_SCANAFTEROPEN);
    listIDs.append(XOptions::ID_SAVELASTDIRECTORY);
    listIDs.append(XOptions::ID_LASTDIRECTORY);
    listIDs.append(XOptions::ID_SAVEBACKUP);

    xOptions.setValueIDs(listIDs);
    xOptions.load();

    adjust();

    if(QCoreApplication::arguments().count()>1)
    {
        QString sFileName=QCoreApplication::arguments().at(1);

        processFile(sFileName,true);
    }
}

GuiMainWindow::~GuiMainWindow()
{
    closeCurrentFile();
    xOptions.save();

    delete ui;
}

void GuiMainWindow::on_actionOpen_triggered()
{
    QString sDirectory=xOptions.getLastDirectory();

    QString sFileName=QFileDialog::getOpenFileName(this,tr("Open file..."),sDirectory,tr("All files (*)"));

    if(!sFileName.isEmpty())
    {
        processFile(sFileName,xOptions.getValue(XOptions::ID_SCANAFTEROPEN).toBool());
    }
}

void GuiMainWindow::on_actionClose_triggered()
{
    closeCurrentFile();
}

void GuiMainWindow::on_actionExit_triggered()
{
    this->close();
}

void GuiMainWindow::on_actionOptions_triggered()
{
    DialogOptions dialogOptions(this,&xOptions);
    dialogOptions.exec();

    adjust();
}

void GuiMainWindow::on_actionAbout_triggered()
{
    DialogAbout dialogAbout(this);
    dialogAbout.exec();
}

void GuiMainWindow::adjust()
{
    xOptions.adjustStayOnTop(this);
}

void GuiMainWindow::processFile(QString sFileName, bool bReload)
{
    if((sFileName!="")&&(QFileInfo(sFileName).isFile()))
    {
        xOptions.setLastDirectory(QFileInfo(sFileName).absolutePath());

        closeCurrentFile();

        pFile=new QFile;

        pFile->setFileName(sFileName);

        if(!pFile->open(QIODevice::ReadWrite))
        {
            if(!pFile->open(QIODevice::ReadOnly))
            {
                closeCurrentFile();
            }
        }

        if(pFile)
        {
            XELF elf(pFile);
            if(elf.isValid())
            {
                pWidget=new ELFWidget(this);

                formatOptions.bIsImage=false;
                formatOptions.nImageBase=-1;
                formatOptions.sBackupFileName=XBinary::getBackupName(pFile);
                pWidget->setData(pFile,&formatOptions,0,0,0);

                if(bReload)
                {
                    pWidget->reload();
                }

                ui->widgetLayot->addWidget(pWidget);

                setWindowTitle(sFileName);
                ui->stackedWidget->setCurrentIndex(1);
            }
            else
            {
                QMessageBox::critical(this,tr("Error"),tr("It is not a valid ELF file!"));
            }
        }
        else
        {
            QMessageBox::critical(this,tr("Error"),tr("Cannot open the file!"));
        }
    }
}

void GuiMainWindow::closeCurrentFile()
{
    if(pWidget)
    {
        delete pWidget;
        pWidget=nullptr;
    }

    if(pFile)
    {
        pFile->close();
        delete pFile;
        pFile=nullptr;
    }

    ui->stackedWidget->setCurrentIndex(0);

    setWindowTitle(QString("%1 v%2").arg(X_APPLICATIONNAME).arg(X_APPLICATIONVERSION));
}

void GuiMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void GuiMainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void GuiMainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData* mimeData=event->mimeData();

    if(mimeData->hasUrls())
    {
        QList<QUrl> urlList=mimeData->urls();

        if(urlList.count())
        {
            QString sFileName=urlList.at(0).toLocalFile();

            sFileName=XBinary::convertFileName(sFileName);

            processFile(sFileName,xOptions.getValue(XOptions::ID_SCANAFTEROPEN).toBool());
        }
    }
}
