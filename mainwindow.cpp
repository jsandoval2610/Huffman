#include "mainwindow.h"
#include <QtWidgets>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), byteFrequencies(256,0)
{
    QWidget *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    fullLayout = new QVBoxLayout();

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    load = new QPushButton("Load");
    buttonLayout->addWidget(load);
    connect(load, &QPushButton::clicked, this, &MainWindow::loadClicked);

    encode = new QPushButton("Encode");
    buttonLayout->addWidget(encode);
    connect(encode, &QPushButton::clicked, this, &MainWindow::encodeClicked);

    decode = new QPushButton("Decode");
    buttonLayout->addWidget(decode);
    connect(decode, &QPushButton::clicked, this, &MainWindow::decodeClicked);


    fileSizes = new QHBoxLayout();

    originalFileLabel = new QLabel("Original File Size (KB): ");
    originalFileSizeLabel = new QLabel("0");

    encodedFileLabel = new QLabel("Encoded File Size (KB): ");
    encodedFileSizeLabel = new QLabel("0");

    decodedFileLabel = new QLabel("Decoded File Size (KB): ");
    decodedFileSizeLabel = new QLabel("0");


    fileSizes->addWidget(originalFileLabel);
    fileSizes->addWidget(originalFileSizeLabel);
    fileSizes->addWidget(encodedFileLabel);
    fileSizes->addWidget(encodedFileSizeLabel);
    fileSizes->addWidget(decodedFileLabel);
    fileSizes->addWidget(decodedFileSizeLabel);

    fullLayout->addLayout(buttonLayout);
    fullLayout->addLayout(fileSizes);


    table = new QTableWidget(256, 4);
    fullLayout->addWidget(table);

    table->setShowGrid(true);
    table->setStyleSheet("QTableView { gridline-color: gray; }");

    centralWidget->setLayout(fullLayout);

    decodedFileLabel->hide();
    decodedFileSizeLabel->hide();

    QStringList headers;
    headers << "Character Code" << "Symbol" << "Frequency" << "Huffman";
    table->setHorizontalHeaderLabels(headers);

    toDo = new QMultiMap<int, QByteArray>();
    children = new QMap<QByteArray, QPair<QByteArray, QByteArray>>();
}

MainWindow::~MainWindow() {}


void MainWindow::loadClicked() {
    QString fName = QFileDialog::getOpenFileName(this, "Open File");
    if(fName.isEmpty()) return;

    QFile in(fName);
    if(!in.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(fName));
        return;
    }

    fileSize = in.size();

    originalFileSizeLabel->setText(QString::number((unsigned int) fileSize / 1024));
    originalFileLabel->show();
    originalFileSizeLabel->show();
    encodedFileLabel->hide();
    encodedFileSizeLabel->hide();
    decodedFileLabel->hide();
    decodedFileSizeLabel->hide();

    fileData = in.readAll();
    if(fileData.isEmpty()) {
        QMessageBox::information(this, "Empty file", "Your file is empty, no point encoding it!");
        return;
    }

    for(int iPos = 0; iPos < fileData.length(); ++iPos) {
        // Increment the frequency of the current byte
        ++byteFrequencies[(unsigned char) fileData[iPos]];
    }

    for(int code = 0; code < 256; ++code) {
        if(byteFrequencies[code]) {

            // Populate the QMultiMap for Huffman
            toDo->insert(byteFrequencies[code], QByteArray(1, code));

            // Column 1: Character Code
            QTableWidgetItem *charCodeItem = new QTableWidgetItem(QString::number(code, 10));

            // Column 2: Symbol (if printable)
            QString symbol = (code >= 32 && code <= 126) ? QString(QChar(code)) : ".";
            QTableWidgetItem *symbolItem = new QTableWidgetItem(symbol);

            // Column 3: Count (frequency)
            QTableWidgetItem *countItem = new QTableWidgetItem(QString::number(byteFrequencies[code]));

            // Populate the rows with these items
            table->setItem(code, 0, charCodeItem);
            table->setItem(code, 1, symbolItem);
            table->setItem(code, 2, countItem);
        }
    }

    load->setEnabled(false);
    encode->setEnabled(true);
    decode->setEnabled(false);
}













void MainWindow::encodeClicked() {

    // Check for unique char edge case
    int uniqueChars = 0;
    for (int i = 0; i < byteFrequencies.size(); ++i) {
        if (byteFrequencies[i] > 0) {
            ++uniqueChars;
        }
    }
    if (uniqueChars <= 1) {
        QMessageBox::information(this, "Cannot Encode", "File contains only one unique character. Huffman encoding is not possible.");
        return;
    }


    while(toDo->size() > 1) {

        const QMultiMap<int, QByteArray>::iterator first = toDo->begin();
        int freq1 = first.key();
        QByteArray byteSeq1 = first.value();
        toDo->erase(first);

        const QMultiMap<int, QByteArray>::iterator second = toDo->begin();
        int freq2 = second.key();
        QByteArray byteSeq2 = second.value();
        toDo->erase(second);

        children->insert(byteSeq1 + byteSeq2, qMakePair(byteSeq1, byteSeq2));
        toDo->insert(freq1 + freq2, byteSeq1 + byteSeq2);

    }


    QByteArray root = toDo->begin().value();
    for(int i = 0; i < 256; ++i) {
        if(byteFrequencies[i] > 0) {
            QString code;
            QByteArray current = root;
            QByteArray target(1, i);  // The byte we're encoding

            // Traverse the tree until we find the target byte
            while(current != target) {
                if(children->value(current).first.contains(target)) {
                    code.append('0');
                    current = children->value(current).first;
                } else {
                    code.append('1');
                    current = children->value(current).second;
                }
            }

            // Store the resulting code for this byte
            decodingHuffmanCodes.insert(code, target);
            encodingHuffmanCodes.insert(target, code);
            QTableWidgetItem *huffCode = new QTableWidgetItem(code);
            table->setItem(i, 3, huffCode);
        }
    }

    for(int iPos = 0; iPos < fileData.length(); ++iPos) {
        allEncodings += encodingHuffmanCodes.value(QByteArray(1, fileData[iPos]));
    }


    // Saving into a file in class
    QString outName = QFileDialog::getSaveFileName(this, "Save", "", "Huffman encoded files (*.huff)");
    if(outName.isEmpty()) {
        return;
    }

    QFile outFile(outName);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \%1\"").arg(outName));
        return;
    }

    QDataStream out(&outFile);
    out << decodingHuffmanCodes;
    out << (qint32)allEncodings.size();

    QByteArray rawBytes;
    for(int i = 0; i < allEncodings.size(); i += 8) {
        char tempByte = allEncodings.mid(i,8).toInt(nullptr, 2);
        rawBytes.append(tempByte);
    }

    out.writeRawData(rawBytes.data(), rawBytes.size());

    encodedFileSize = outFile.size();
    encodedFileSizeLabel->setText(QString::number((unsigned int) encodedFileSize / 1024));

    originalFileLabel->show();
    originalFileSizeLabel->show();
    encodedFileLabel->show();
    encodedFileSizeLabel->show();
    decodedFileLabel->hide();
    decodedFileSizeLabel->hide();

    outFile.close();
    encode->setEnabled(false);
    decode->setEnabled(true);

}






void MainWindow::decodeClicked() {

    table->clear();
    byteFrequencies.fill(0);
    fileSize = 0;
    table->setColumnCount(3);

    QStringList headers;
    headers << "Character Code" << "Symbol" << "Huffman";
    table->setHorizontalHeaderLabels(headers);


    // Load the binary file
    QString binaryInFile = QFileDialog::getOpenFileName(this, "Open encoded file", "");
    if(binaryInFile.isEmpty()) return;

    QFile in(binaryInFile);
    if(!in.open(QIODevice::ReadOnly)) {
        QMessageBox::information(this, "Error", QString("Can't open file \"%1\"").arg(binaryInFile));
        return;
    }

    fileSize = in.size();
    originalFileSizeLabel->setText(QString::number((unsigned int) fileSize / 1024));


    QDataStream inStream(&in);
    inStream >> decodingHuffmanCodes2;


    inStream >> encodedDataSize;


    // Filling out table
    int count = 0;
    for(auto it = decodingHuffmanCodes2.constBegin(); it != decodingHuffmanCodes2.constEnd(); ++it) {

        QTableWidgetItem *charCodeItem = new QTableWidgetItem();
        charCodeItem->setData(Qt::DisplayRole, (unsigned char) it.value()[0]);

        int temp = (unsigned char) it.value()[0];

        QString symbol = (temp >= 32 && temp <= 126) ? QString(QChar(temp)) : ".";
        QTableWidgetItem *symbolItem = new QTableWidgetItem(symbol);

        QTableWidgetItem *huffCode = new QTableWidgetItem(it.key());

        table->setItem(count, 0, charCodeItem);
        table->setItem(count, 1, symbolItem);
        table->setItem(count, 2, huffCode);

        ++count;
    }


    encodedBytes.resize((encodedDataSize + 7) / 8);

    inStream.readRawData(encodedBytes.data(),encodedDataSize);

    // Convert the bytes back to bits
    for (int i = 0; i < encodedBytes.size(); ++i) {
        int width = qMin(encodedDataSize, 8);
        encodedBits.append(QString::number(((unsigned char)encodedBytes[i]), 2).rightJustified(width,'0'));
        encodedDataSize -= width;
    }


    //Decode using Huffman map
    QString curr = "";
    QByteArray decodedData;
    for(int i = 0; i < encodedBits.length(); ++i) {
        curr.append(encodedBits[i]);
        if (decodingHuffmanCodes2.contains(curr)) {
            decodedData.append(decodingHuffmanCodes2.value(curr));
            curr.clear();
        }
    }


    // Save the decoded file
    QString outName = QFileDialog::getSaveFileName(this, "Save Decoded File", "");
    if(outName.isEmpty()) return;


    QFile outFile(outName);
    if(!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(this, "Error", QString("Can't write to file \"%1\"").arg(outName));
        return;
    }


    QDataStream out(&outFile);

    out.writeRawData(decodedData.data(), decodedData.size());
    outFile.close();

    decodedFileSizeLabel->setText(QString::number((unsigned int) decodedData.size() / 1024));

    originalFileLabel->show();
    originalFileSizeLabel->show();
    encodedFileLabel->hide();
    encodedFileSizeLabel->hide();
    decodedFileLabel->show();
    decodedFileSizeLabel->show();

    load->setEnabled(false);
    encode->setEnabled(false);
    decode->setEnabled(false);

}
