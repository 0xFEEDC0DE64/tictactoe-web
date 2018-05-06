#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>

enum class FieldType { EMPTY, CIRCLE, CROSS };

char fieldTypeLetter(const FieldType fieldType)
{
    switch(fieldType)
    {
    case FieldType::EMPTY: return '_';
    case FieldType::CIRCLE: return 'O';
    case FieldType::CROSS: return 'X';
    default: qFatal("unexpected field type");
    }
}

template<std::size_t WIDTH, std::size_t HEIGHT>
QString getFieldName(const std::array<std::array<FieldType, WIDTH>, HEIGHT> &field)
{
    QString result;

    for(int y = 0; y < HEIGHT; y++)
    {
        const auto &row = field.at(y);

        if(!result.isEmpty())
            result.append('-');

        for(int x = 0; x < WIDTH; x++)
            result.append(fieldTypeLetter(row.at(x)));
    }

    return result;
}

template<std::size_t WIDTH, std::size_t HEIGHT>
quint64 getFieldHash(const std::array<std::array<FieldType, WIDTH>, HEIGHT> &field)
{
    quint64 result = 0;
    quint64 digitValue = 1;

    for(quint32 y = 0; y < HEIGHT; y++)
        for(quint32 x = 0; x < WIDTH; x++)
        {
            quint8 digit = (quint8)field.at(y).at(x);

            result += digit * digitValue;

            digitValue *= 3;
        }

    return result;
}

template<std::size_t SIZE>
FieldType getFieldWinner(const std::array<std::array<FieldType, SIZE>, SIZE> &field)
{
    for(quint32 y = 0; y < SIZE; y++)
    {
        const auto &row = field.at(y);
        auto last = row.at(0);

        if(last != FieldType::EMPTY)
        {
            auto failed = false;
            for(quint32 x = 1; x < SIZE; x++)
            {
                if(row.at(x) != last)
                {
                    failed = true;
                    break;
                }
                last = row.at(x);
            }

            if(!failed)
                return last;
        }
    }

    for(quint32 x = 0; x < SIZE; x++)
    {
        auto last = field.at(0).at(SIZE-1);

        if(last != FieldType::EMPTY)
        {
            auto failed = false;
            for(quint32 y = 1; y < SIZE; y++)
            {
                if(field.at(y).at(x) != last)
                {
                    failed = true;
                    break;
                }
                last = field.at(y).at(x);
            }

            if(!failed)
                return last;
        }
    }

    {
        auto last = field.at(0).at(0);

        if(last != FieldType::EMPTY)
        {
            auto failed = false;
            for(quint32 i = 1; i < SIZE; i++)
            {
                if(field.at(i).at(i) != last)
                {
                    failed = true;
                    break;
                }
                last = field.at(i).at(i);
            }

            if(!failed)
                return last;
        }
    }

    {
        auto last = field.at(0).at(SIZE-1);

        if(last != FieldType::EMPTY)
        {
            auto failed = false;
            for(quint32 i = 1; i < SIZE; i++)
            {
                if(field.at(i).at(SIZE - i) != last)
                {
                    failed = true;
                    break;
                }
                last = field.at(i).at(SIZE - i);
            }

            if(!failed)
                return last;
        }
    }

    return FieldType::EMPTY;
}

template<std::size_t WIDTH, std::size_t HEIGHT>
QString writeField(const QDir &dir, const std::array<std::array<FieldType, WIDTH>, HEIGHT> &field)
{
    const auto fieldHash = getFieldHash(field);
    const auto name = (fieldHash == 0 ? QStringLiteral("index") : QString::number(fieldHash)) + ".html";

    qDebug() << "writing" << name;

    QFile file(dir.absoluteFilePath(name));

    if(file.exists())
        return name;

    auto winner = getFieldWinner(field);

    const auto fieldPlayer = std::count(std::begin(field.at(0)), std::end(field.at(2)), FieldType::CIRCLE) <
                             std::count(std::begin(field.at(0)), std::end(field.at(2)), FieldType::CROSS) ?
                             FieldType::CIRCLE : FieldType::CROSS;

    if(!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        qFatal("Could not open file");

    QTextStream textStream(&file);
    textStream << "<table border=\"1\">" << endl;

    for(quint32 y = 0; y < HEIGHT; y++)
    {
        const auto &row = field.at(y);

        textStream << "<tr>" << endl;

        for(quint32 x = 0; x < WIDTH; x++)
        {
            textStream << "<td>";

            if(row.at(x) == FieldType::EMPTY && winner == FieldType::EMPTY)
            {
                auto copy = field;
                copy[y][x] = fieldPlayer;
                const auto otherName = writeField(dir, copy);
                textStream << "<a href=\"" << otherName << "\">_</a>";
            }
            else
                textStream << fieldTypeLetter(row.at(x));

            textStream << "</td>" << endl;
        }

        textStream << "</tr>" << endl;
    }

    textStream << "</table>" << endl;

    switch(winner)
    {
    case FieldType::CIRCLE:
        textStream << "<span style=\"font-size: 2em;\">CIRCLE won</span>" << endl;
        break;
    case FieldType::CROSS:
        textStream << "<span style=\"font-size: 2em;\">CROSS won</span>" << endl;
        break;
    default:
        switch(fieldPlayer)
        {
        case FieldType::CIRCLE:
            textStream << "<span style=\"font-size: 2em;\">CIRCLE plays</span>" << endl;
            break;
        case FieldType::CROSS:
            textStream << "<span style=\"font-size: 2em;\">CROSS plays</span>" << endl;
            break;
        default:
            qt_noop();
        }
    }

    return name;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QDir dir("files");
    dir.removeRecursively();
    dir.mkpath(dir.absolutePath());

    std::array<std::array<FieldType, 3>, 3> test {};
    writeField(dir, test);

    return 0;
}

