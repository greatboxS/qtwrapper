bool hiraganaLessThan(const QString &s1, const QString &s2) {
    for (int i = 0; i < s1.size() && i < s2.size(); i++) {
        QChar c1 = s1.at(i);
        QChar c2 = s2.at(i);
        ushort u1 = c1.unicode();
        ushort u2 = c2.unicode();

        if (u1 < 128 && u2 < 128) {
            if (u1 != u2) {
                return (u1 < u2);
            }
        } else if (u1 >= 0x3040 && u1 <= 0x309F && u2 >= 0x3040 && u2 <= 0x309F) {
            if ((u1 + 1 == u2) || (u1 == u2 + 1)) {
                // Uppercase hiragana character is less than lowercase
                return (u1 > u2);
            }
            // Otherwise, use standard lexicographical order
            else if (u1 != u2) {
                return (u1 < u2);
            }
        } else {
            // Swap return values to sort ASCII characters after hiragana characters
            return (u2 < u1);
        }
    }
    return (s1.size() < s2.size());
}

int main(int argc, char const *argv[]) {
    QStringList list;
    list << "a"
         << "いろ"
         << "b"
         << "あかい"
         << "ぁか"
         << "アカ"
         << "イロ"
         << "c"
         << "あかみどり"
         << "ぇ"
         << "え"
         << "が"
         << "き";

    qInfo() << "Before sorting:" << list;

    std::sort(list.begin(), list.end(), hiraganaLessThan);

    qInfo() << "After sorting:" << list;

    qInfo() << ("ア" > "あ");
    QString s1("ア");
    QString s2("あ");

    qInfo() << s1.size() << " " << s2.size();
    for (int i = 0; i < s1.size(); i++) {
        QChar c1 = s1.at(i);
        qInfo() << c1;
    }

    for (int i = 0; i < s2.size(); i++) {
        QChar c1 = s2.at(i);
        qInfo() << c1;
    }

    return 0;
}