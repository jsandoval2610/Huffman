#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QtWidgets>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTableWidget *table;
    QPushButton *load;
    QPushButton *encode;
    QPushButton *decode;
    QHBoxLayout *fileSizes;
    QVBoxLayout *fullLayout;

    QVector<int> byteFrequencies;
    QByteArray fileData;
    QMultiMap<int, QByteArray> *toDo; // Maps a frequency to the QByteArray it corresponds to
    QMap<QByteArray, QPair<QByteArray, QByteArray> > *children;
    QMap<QString, QByteArray> decodingHuffmanCodes;
    QMap<QByteArray, QString> encodingHuffmanCodes;
    QString allEncodings;

    QMap<QString, QByteArray> decodingHuffmanCodes2;
    QByteArray encodedBytes;
    QString encodedBits;
    int encodedDataSize;

    int fileSize;
    int encodedFileSize;
    int decodedFileSize;

    QLabel *originalFileSizeLabel;
    QLabel *encodedFileSizeLabel;
    QLabel *decodedFileSizeLabel;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


public slots:
    void loadClicked();
    void encodeClicked();
    void decodeClicked();


};

#endif // MAINWINDoW
