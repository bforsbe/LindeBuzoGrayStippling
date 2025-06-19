/*
 *      This is an interactive demo application for the algorithm proposed in:
 *
 *      Weighted Linde-Buzo Gray Stippling
 *      Oliver Deussen, Marc Spicker, Qian Zheng
 *
 *      In: ACM Transactions on Graphics (Proceedings of SIGGRAPH Asia 2017)
 *      https://doi.org/10.1145/3130800.3130819
 *
 *     Copyright 2017 Marc Spicker (marc.spicker@googlemail.com)
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QImage>
#include <QString>
#include <QVector2D>
#include <iostream>
#include <fstream>

#include "mainwindow.h"
#include "stippleviewer.h"

// Binary save function for stipples
bool binarySaveRaw(const std::string &path, const std::vector<QVector2D> &pts) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    uint64_t n = pts.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    out.write(reinterpret_cast<const char*>(pts.data()), n * sizeof(QVector2D));
    return out.good();
}

using Params = LBGStippling::Params;
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Weighted LBG Stippling");

    QCommandLineParser parser;
    parser.setApplicationDescription("Weighted Linde‑Buzo‑Gray Stippling CLI");
    parser.addHelpOption();

    QCommandLineOption inputOpt({"i", "input"}, "Input image file path", "input");
    QCommandLineOption outputOpt({"o", "output"}, "Output file path (.png, .jpg, .raw)", "output");

    parser.addOption(inputOpt);
    parser.addOption(outputOpt);

    // Optional parameters
    parser.addOption({"points", "Initial number of points", "int", "1"});
    parser.addOption({"pointSize", "Initial point size. Fixed unless sizeMin and SizeMax are set > 0.0", "int", "2"});
    parser.addOption({"sizeMin", "Min point size", "float", "-1.0"});
    parser.addOption({"sizeMax", "Max point size", "float", "-1.0"});
    parser.addOption({"ss", "Supersampling factor", "int", "1"});
    parser.addOption({"iter", "Max iterations", "int", "50"});
    parser.addOption({"hyst", "Hysteresis factor", "float", "0.6"});
    parser.addOption({"hystDelta", "Hysteresis delta", "float", "0.01"});

    parser.process(app);

    const QString inPath = parser.value(inputOpt);
    const QString outPath = parser.value(outputOpt);

    if (parser.isSet(inputOpt)) {     
        if (!QFileInfo::exists(inPath)) {
            std::cerr << "Input file not found: " << inPath.toStdString() << "\n";
            return 1;
        }

        if (inPath.isEmpty() || outPath.isEmpty()) {
            std::cerr << "Both --input and --output are required.\n";
            return 1;
        }

        QFileInfo outInfo(outPath);
        QString ext = outInfo.suffix().toLower();

        if (ext != "png" && ext != "jpg" && ext != "jpeg" && ext != "raw" && ext != "bin") {
            std::cerr << "Unsupported output format: ." << ext.toStdString() << "\n";
            std::cerr << "Supported extensions: .png, .jpg, .jpeg, .raw, .bin\n";
            return 1;
        }

        QImage input(inPath);
        if (input.isNull()) {
            std::cerr << "Failed to load input image: " << inPath.toStdString() << "\n";
            return 1;
        }

        Params params;
        params.initialPoints       = parser.value("points").toULongLong();
        params.initialPointSize    = parser.value("pointSize").toFloat();
        params.pointSizeMin        = parser.value("sizeMin").toFloat();
        params.pointSizeMax        = parser.value("sizeMax").toFloat();
        if (params.pointSizeMin > 0.0f && params.pointSizeMax> 0.0f)
            params.adaptivePointSize = true;
        params.superSamplingFactor = parser.value("ss").toULongLong();
        params.maxIterations       = parser.value("iter").toULongLong();
        params.hysteresis          = parser.value("hyst").toFloat();
        params.hysteresisDelta     = parser.value("hystDelta").toFloat();

        LBGStippling engine;
        auto pts = engine.stipple(input, params);

        StippleViewer viewer(input, nullptr);
        viewer.displayPoints(pts);
        if (ext == "png" || ext == "jpg" || ext == "jpeg") {
            QPixmap outputImage = viewer.getImage();
            if (!outputImage.save(outPath)) {
                std::cerr << "Failed to save output image to: " << outPath.toStdString() << "\n";
                return 1;
            }
        } else {
            std::vector<QVector2D> positions;
            positions.reserve(pts.size()); // avoid reallocations

            std::transform(pts.begin(), pts.end(), std::back_inserter(positions),
                [](const Stipple& s) { return s.pos; });
            if (!binarySaveRaw(outPath.toStdString(), positions)) {
                std::cerr << "Failed to save binary stipple data to: " << outPath.toStdString() << "\n";
                return 1;
            }
        }
        return 0;
    }
    else
    {
        std::cerr << "Input file not specified, launching GUI " << "\n";
        // Default: launch GUI.
        MainWindow window;
        window.show();

        return app.exec();
    }
}