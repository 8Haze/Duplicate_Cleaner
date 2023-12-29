#include "main_window.h"
#include "ui_main_window.h"

#include <fstream>

#include <QFileDialog>
#include <QMessageBox>

#include "hct/sort.h"
#include "hct/scope_guard.h"

// ================================================================
// | Main_Window Class - Constructors                             |
// ================================================================

Main_Window::Main_Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Main_Window)
{
    ui->setupUi(this);

    setWindowTitle("Duplicate Cleaner");

    ui->directory_label->setText(directory_label_prefix + "<none>");
}

// ================================================================
// | Main_Window Class - Private                                  |
// ================================================================

void Main_Window::reset_directory()
{
    directory.clear();
    ui->directory_label->setText(directory_label_prefix + "<none>");
    reset_files();
}

void Main_Window::reset_files()
{
    files.clear();
    ui->files_list->clear();
    total_saved_disk_space = 0;
    ui->saved_disk_space_label->setText(directory_label_prefix + " 0 B");
}

void Main_Window::fill_files_list()
{
    for (size_t a = 0; a < files.size(); ++a)
    {
        auto processed_size = process_bytes(files[a].get_size());
        QString string;
        string.reserve(files[a].get_path().filename().generic_u16string().size() + 30);

        string += QString::number(a + 1);
        string += ". ";
        string += files[a].get_path().relative_path().generic_u16string();
        string += " (";
        string += to_pretty_string(processed_size.first);
        string += ' ';
        string += processed_size.second;
        string += ") ";
        string += (files[a].is_removable() ? "[Removable]" : "[Original]");

        ui->files_list->addItem(string);
        if (files[a].is_removable())
            ui->files_list->item(ui->files_list->count() - 1)->setForeground(Qt::GlobalColor::red);
    }
}

void Main_Window::analyze_files()
{
    hct::sort(files.begin(), files.end());

    QString original_text = this->windowTitle();
    setWindowTitle(original_text + " - Analyzing. Please wait...");

    for (size_t a = 0; a < files.size(); ++a)
        for (size_t b = a + 1; b < files.size(); ++b)
            if (!files[b].is_removable() && (files[a].get_size() == files[b].get_size()) && compare_contents(files[a], files[b]))
                files[b].set_removable(true);

    setWindowTitle(original_text);
}

bool Main_Window::compare_contents(const File& file_1, const File& file_2)
{
    std::ifstream ifs_1(file_1.get_path(), std::ios::binary);
    std::ifstream ifs_2(file_2.get_path(), std::ios::binary);
    auto ifs_1_guard = hct::make_guard_exit([&ifs_1]() { if (ifs_1.is_open()) ifs_1.close(); });
    auto ifs_2_guard = hct::make_guard_exit([&ifs_2]() { if (ifs_2.is_open()) ifs_2.close(); });

    if (!ifs_1.is_open() || !ifs_2.is_open())
        return false;

    char dst_1, dst_2;

    while (true)
    {
        ifs_1.read(&dst_1, sizeof(dst_1));
        ifs_2.read(&dst_2, sizeof(dst_2));

        if (ifs_1.eof() || ifs_2.eof())
            break;

        if (dst_1 != dst_2)
            return false;
    }

    return true;
}

void Main_Window::calculate_and_display_saved_disk_space()
{
    total_saved_disk_space = 0;

    for (const auto& file : files)
        if (file.is_removable())
            total_saved_disk_space += file.get_size();

    auto processed_space = process_bytes(total_saved_disk_space);
    QString string = saved_disk_space_label_prefix;

    string += to_pretty_string(processed_space.first);
    string += ' ';
    string += processed_space.second;
    if (processed_space.second != "B")
    {
        string += " [";
        string += to_pretty_string(total_saved_disk_space);
        string += " bytes]";
    }

    ui->saved_disk_space_label->setText(string);
}

// ================================================================
// | Main_Window Class - Private Slots                            |
// ================================================================

void Main_Window::on_button_choose_directory_clicked()
{
    QString choice = QFileDialog::getExistingDirectory(this, "Select Directory");

    if (choice.isEmpty())
        return;

    directory = choice.toStdString();
    ui->directory_label->setText(directory_label_prefix + directory.string().c_str());

    reset_files();
}

void Main_Window::on_button_run_analysis_clicked()
{
    reset_files();

    if (directory.empty())
        return;

    if (ui->option_recursive->isChecked())
        fetch_files<std::filesystem::recursive_directory_iterator>();
    else
        fetch_files<std::filesystem::directory_iterator>();

    analyze_files();
    fill_files_list();
    calculate_and_display_saved_disk_space();
}

void Main_Window::on_button_remove_duplicates_clicked()
{
    if (files.empty())
        return;

    for (const auto& file : files)
    {
        try
        {
            if (file.is_removable())
                std::filesystem::remove(file.get_path());
        }
        catch (const std::filesystem::filesystem_error&)
        {
            QString error_string = "Could not remove file ";
            error_string += file.get_path().relative_path().generic_u16string();
            QMessageBox::critical(this, "File Removal Error", error_string);
            total_saved_disk_space -= file.get_size();
        }
    }

    auto processed_space = process_bytes(total_saved_disk_space);
    QString string = cleanup_message_box_prefix;

    string += to_pretty_string(processed_space.first);
    string += ' ';
    string += processed_space.second;
    if (processed_space.second != "B")
    {
        string += " [";
        string += to_pretty_string(total_saved_disk_space);
        string += " bytes]";
    }

    reset_directory();

    QMessageBox::information(this, "Cleanup Complete", string);
}

// ================================================================
// | Main_Window Class - Destructor                               |
// ================================================================

Main_Window::~Main_Window()
{
    delete ui;
}
