#include "addlangdialog.h"

#include <QPushButton>

#include "ui_addlangdialog.h"

AddLangDialog::AddLangDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddLangDialog)
{
    ui->setupUi(this);

    // Load languages
    for (int i = 1; i <= QOnlineTranslator::Zulu; ++i) {
        const auto lang = static_cast<QOnlineTranslator::Language>(i);

        auto item = new QListWidgetItem();
        item->setText(QOnlineTranslator::languageString(lang));
        item->setIcon(QIcon(":/icons/flags/" + QOnlineTranslator::languageCode(lang) + ".svg"));
        item->setData(Qt::UserRole, lang);
        ui->langListWidget->addItem(item);
    }
    ui->langListWidget->setCurrentRow(0);
}

AddLangDialog::~AddLangDialog()
{
    delete ui;
}

void AddLangDialog::on_searchEdit_textChanged(const QString &text)
{
    bool isItemSelected = false;
    for (int i = 0; i < ui->langListWidget->count(); ++i) {
        QListWidgetItem *item = ui->langListWidget->item(i);
        if (item->text().contains(text)) {
            item->setHidden(false);
            if (!isItemSelected) {
                item->setSelected(true); // Select first unhidden item
                isItemSelected = true;
            }
        } else {
            item->setHidden(true);
        }
    }

    // Disable Ok button if no item selected
    ui->dialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(isItemSelected);
}

QOnlineTranslator::Language AddLangDialog::language() const
{
    return m_lang;
}

void AddLangDialog::on_dialogButtonBox_accepted()
{
    m_lang = ui->langListWidget->currentItem()->data(Qt::UserRole).value<QOnlineTranslator::Language>();
}
