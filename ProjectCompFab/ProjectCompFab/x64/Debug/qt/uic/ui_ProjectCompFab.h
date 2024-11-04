/********************************************************************************
** Form generated from reading UI file 'ProjectCompFab.ui'
**
** Created by: Qt User Interface Compiler version 6.7.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROJECTCOMPFAB_H
#define UI_PROJECTCOMPFAB_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ProjectCompFabClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ProjectCompFabClass)
    {
        if (ProjectCompFabClass->objectName().isEmpty())
            ProjectCompFabClass->setObjectName("ProjectCompFabClass");
        ProjectCompFabClass->resize(600, 400);
        menuBar = new QMenuBar(ProjectCompFabClass);
        menuBar->setObjectName("menuBar");
        ProjectCompFabClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ProjectCompFabClass);
        mainToolBar->setObjectName("mainToolBar");
        ProjectCompFabClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(ProjectCompFabClass);
        centralWidget->setObjectName("centralWidget");
        ProjectCompFabClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(ProjectCompFabClass);
        statusBar->setObjectName("statusBar");
        ProjectCompFabClass->setStatusBar(statusBar);

        retranslateUi(ProjectCompFabClass);

        QMetaObject::connectSlotsByName(ProjectCompFabClass);
    } // setupUi

    void retranslateUi(QMainWindow *ProjectCompFabClass)
    {
        ProjectCompFabClass->setWindowTitle(QCoreApplication::translate("ProjectCompFabClass", "ProjectCompFab", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ProjectCompFabClass: public Ui_ProjectCompFabClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROJECTCOMPFAB_H
