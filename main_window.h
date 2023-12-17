#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <vector>

#include <QMainWindow>

#include "file.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Main_Window; }
QT_END_NAMESPACE

class Main_Window : public QMainWindow
{
private:

    Q_OBJECT

    inline static const QString directory_label_prefix = "Selected directory: ";
    inline static const QString saved_disk_space_label_prefix = "Disk space that can be freed: ";
    inline static const QString cleanup_message_box_prefix = "Cleanup complete. Total saved disk space: ";

    Ui::Main_Window *ui;

    std::filesystem::path directory;
    std::vector<File> files;
    size_t total_saved_disk_space = 0;

    template <typename Dir_Iter>
    void fetch_files()
    {
        if (directory.empty())
            return;

        for (const auto& entry : Dir_Iter(directory))
            if (entry.is_regular_file())
                files.emplace_back(entry.path(), entry.file_size(), false);
    }

    void reset_directory();
    void reset_files();
    void fill_files_list();
    void analyze_files();
    bool compare_contents(const File& file1, const File& file2);
    void calculate_and_display_saved_disk_space();

public:

    Main_Window(QWidget *parent = nullptr);

    ~Main_Window();

private slots:

    void on_button_choose_directory_clicked();
    void on_button_run_analysis_clicked();
    void on_button_remove_duplicates_clicked();

};

#endif // MAIN_WINDOW_H
