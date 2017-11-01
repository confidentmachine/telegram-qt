#include <QObject>
#include <QTest>
#include <QDebug>

#include "GeneratorNG.hpp"

const QString c_typesSection = QStringLiteral("---types---");

const QStringList c_sourcesRichText =
{
    QStringLiteral("textEmpty#dc3d824f = RichText;"),
    QStringLiteral("textPlain#744694e0 text:string = RichText;"),
    QStringLiteral("textBold#6724abc4 text:RichText = RichText;"),
    QStringLiteral("textItalic#d912a59c text:RichText = RichText;"),
    QStringLiteral("textUnderline#c12622c4 text:RichText = RichText;"),
    QStringLiteral("textStrike#9bf8bb95 text:RichText = RichText;"),
    QStringLiteral("textFixed#6c3f19b9 text:RichText = RichText;"),
    QStringLiteral("textUrl#3c2884c1 text:RichText url:string webpage_id:long = RichText;"),
    QStringLiteral("textEmail#de5a0dd6 text:RichText email:string = RichText;"),
    QStringLiteral("textConcat#7e6260d7 texts:Vector<RichText> = RichText;"),
};

const QStringList c_sourcesInputMedia =
{
    QStringLiteral("inputMediaEmpty#9664f57f = InputMedia;"),
    QStringLiteral("inputMediaUploadedPhoto#f7aff1c0 file:InputFile caption:string = InputMedia;"),
    QStringLiteral("inputMediaPhoto#e9bfb4f3 id:InputPhoto caption:string = InputMedia;"),
    QStringLiteral("inputMediaGeoPoint#f9c44144 geo_point:InputGeoPoint = InputMedia;"),
    QStringLiteral("inputMediaContact#a6e45987 phone_number:string first_name:string last_name:string = InputMedia;"),
    QStringLiteral("inputMediaUploadedVideo#82713fdf file:InputFile duration:int w:int h:int mime_type:string caption:string = InputMedia;"),
    QStringLiteral("inputMediaUploadedThumbVideo#7780ddf9 file:InputFile thumb:InputFile duration:int w:int h:int mime_type:string caption:string = InputMedia;"),
    QStringLiteral("inputMediaVideo#936a4ebd id:InputVideo caption:string = InputMedia;"),
    QStringLiteral("inputMediaUploadedAudio#4e498cab file:InputFile duration:int mime_type:string = InputMedia;"),
    QStringLiteral("inputMediaAudio#89938781 id:InputAudio = InputMedia;"),
    QStringLiteral("inputMediaUploadedDocument#1d89306d file:InputFile mime_type:string attributes:Vector<DocumentAttribute> caption:string = InputMedia;"),
    QStringLiteral("inputMediaUploadedThumbDocument#ad613491 file:InputFile thumb:InputFile mime_type:string attributes:Vector<DocumentAttribute> caption:string = InputMedia;"),
    QStringLiteral("inputMediaDocument#1a77f29c id:InputDocument caption:string = InputMedia;"),
    QStringLiteral("inputMediaVenue#2827a81a geo_point:InputGeoPoint title:string address:string provider:string venue_id:string = InputMedia;"),
    QStringLiteral("inputMediaGifExternal#4843b0fd url:string q:string = InputMedia;"),
};

static QByteArray mkTextSpecTypes(const QStringList &sources)
{
    const QString textData = c_typesSection + QLatin1Char('\n') + sources.join(QLatin1Char('\n')) + QLatin1Char('\n');
    return textData.toLocal8Bit();
}

class tst_GeneratorNG : public QObject
{
    Q_OBJECT
public:
    explicit tst_GeneratorNG(QObject *parent = nullptr);
private slots:
    void checkRemoveWord_data();
    void checkRemoveWord();
    void checkInputMedia();
    void checkTypeSources();
    void recursiveType();
    void recursiveTypeMembers();
    void typeWithVector();
};

tst_GeneratorNG::tst_GeneratorNG(QObject *parent) :
    QObject(parent)
{
}

void tst_GeneratorNG::checkRemoveWord_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("output");
    QTest::addColumn<QString>("word");

    QTest::newRow("No word in the input")
            << "idInputVideo"
            << "idInputVideo"
            << "audio";
    QTest::newRow("Word in the end (different case)")
            << "idInputVideo"
            << "idInput"
            << "video";
    QTest::newRow("Word in the end (same case)")
            << "idInputVideo"
            << "idInput"
            << "Video";
    QTest::newRow("Word in the middle (different case)")
            << "idInputVideo"
            << "idVideo"
            << "input";
    QTest::newRow("Word in the middle (same case)")
            << "idInputVideo"
            << "idVideo"
            << "Input";
    QTest::newRow("Word in the beginning (different case)")
            << "idInputVideo"
            << "inputVideo"
            << "Id";
    QTest::newRow("Word in the beginning (same case)")
            << "idInputVideo"
            << "inputVideo"
            << "id";
}

void tst_GeneratorNG::checkRemoveWord()
{
    QFETCH(QString, input);
    QFETCH(QString, output);
    QFETCH(QString, word);
    QCOMPARE(GeneratorNG::removeWord(input, word), output);
}

void tst_GeneratorNG::checkInputMedia()
{
    const QStringList sources = c_sourcesInputMedia;
    const QByteArray textData = mkTextSpecTypes(sources);
    GeneratorNG generator;
    QVERIFY(generator.loadFromText(textData));
    QMap<QString, TLType> types = generator.types();
    QCOMPARE(types.size(), 1);

//    QVERIFY(generator.resolveTypes());

//    QVERIFY(!generator.solvedTypes().isEmpty());
//    const TLType solvedType = generator.solvedTypes().first();
    const TLType solvedType = types.first();
    const QStringList structMembers = GeneratorNG::generateTLTypeMembers(solvedType);
    qDebug().noquote() << structMembers.join(QLatin1Char('\n'));

    QVERIFY(structMembers.contains(QStringLiteral("TLInputFile thumb;")));
    QVERIFY(structMembers.contains(QStringLiteral("TLInputVideo idInputVideo;")));
    QVERIFY(structMembers.contains(QStringLiteral("TLInputAudio idInputAudio;")));
    QVERIFY(structMembers.contains(QStringLiteral("TLVector<TLDocumentAttribute> attributes;")));
}

void tst_GeneratorNG::checkTypeSources()
{
    const QStringList sources =
    {
        QStringLiteral("inputGeoPointEmpty#e4c123d6 = InputGeoPoint;"),
        QStringLiteral("inputGeoPoint#f3b7acc9 lat:double long:double = InputGeoPoint;"),
    };
    const QString textData = c_typesSection + QLatin1Char('\n') + sources.join(QLatin1Char('\n')) + QLatin1Char('\n');
    GeneratorNG generator;
    QVERIFY(generator.loadFromText(textData.toLocal8Bit()));
    const QMap<QString, TLType> types = generator.types();
    QCOMPARE(types.size(), 1);
    const TLType t = types.first();
    QCOMPARE(t.name, QStringLiteral("TLInputGeoPoint"));
    QCOMPARE(t.subTypes.size(), sources.size());
    for (int i = 0; i < t.subTypes.size(); ++i) {
        const TLSubType &subtype = t.subTypes.at(i);
        QCOMPARE(subtype.source, sources.at(i));
    }
}

void tst_GeneratorNG::recursiveType()
{
    return;
    const QStringList sources = c_sourcesRichText;
    const QByteArray textData = mkTextSpecTypes(sources);
    GeneratorNG generator;
    QVERIFY(generator.loadFromText(textData));
    QMap<QString, TLType> types = generator.types();
    QCOMPARE(types.size(), 1);

    QVERIFY(generator.resolveTypes());

    QVERIFY(!generator.solvedTypes().isEmpty());
    const TLType solvedType = generator.solvedTypes().first();
    const QString source = GeneratorNG::generateTLTypeDefinition(solvedType);
    qDebug().noquote() << source;
}

void tst_GeneratorNG::recursiveTypeMembers()
{
    const QStringList sources = c_sourcesRichText;
    const QByteArray textData = mkTextSpecTypes(sources);
    GeneratorNG generator;
    QVERIFY(generator.loadFromText(textData));
    QMap<QString, TLType> types = generator.types();
    QCOMPARE(types.size(), 1);

    QVERIFY(generator.resolveTypes());

    QVERIFY(!generator.solvedTypes().isEmpty());
    const TLType solvedType = generator.solvedTypes().first();
    const QStringList structMembers = GeneratorNG::generateTLTypeMembers(solvedType);
    qDebug().noquote() << structMembers.join(QLatin1Char('\n'));

    QVERIFY(structMembers.contains(QStringLiteral("QString email;")));
    QVERIFY(structMembers.contains(QStringLiteral("QString textString;")));
    QVERIFY(structMembers.contains(QStringLiteral("TLRichText *textRich;")));
    QVERIFY(structMembers.contains(QStringLiteral("TLVector<TLRichText*> texts;")));
}

void tst_GeneratorNG::typeWithVector()
{
    return;
    const QStringList sources =
    {
        QStringLiteral("pageBlockUnsupported#13567e8a = PageBlock;"),
        QStringLiteral("pageBlockTitle#70abc3fd text:RichText = PageBlock;"),
        QStringLiteral("pageBlockSubtitle#8ffa9a1f text:RichText = PageBlock;"),
        QStringLiteral("pageBlockAuthorDate#baafe5e0 author:RichText published_date:int = PageBlock;"),
        QStringLiteral("pageBlockHeader#bfd064ec text:RichText = PageBlock;"),
        QStringLiteral("pageBlockSubheader#f12bb6e1 text:RichText = PageBlock;"),
        QStringLiteral("pageBlockParagraph#467a0766 text:RichText = PageBlock;"),
        QStringLiteral("pageBlockPreformatted#c070d93e text:RichText language:string = PageBlock;"),
        QStringLiteral("pageBlockFooter#48870999 text:RichText = PageBlock;"),
        QStringLiteral("pageBlockDivider#db20b188 = PageBlock;"),
        QStringLiteral("pageBlockAnchor#ce0d37b0 name:string = PageBlock;"),
        QStringLiteral("pageBlockList#3a58c7f4 ordered:Bool items:Vector<RichText> = PageBlock;"),
        QStringLiteral("pageBlockBlockquote#263d7c26 text:RichText caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockPullquote#4f4456d3 text:RichText caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockPhoto#e9c69982 photo_id:long caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockVideo#d9d71866 flags:# autoplay:flags.0?true loop:flags.1?true video_id:long caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockCover#39f23300 cover:PageBlock = PageBlock;"),
        QStringLiteral("pageBlockEmbed#cde200d1 flags:# full_width:flags.0?true allow_scrolling:flags.3?true url:flags.1?string html:flags.2?string "
        "poster_photo_id:flags.4?long w:int h:int caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockEmbedPost#292c7be9 url:string webpage_id:long author_photo_id:long author:string "
        "date:int blocks:Vector<PageBlock> caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockCollage#8b31c4f items:Vector<PageBlock> caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockSlideshow#130c8963 items:Vector<PageBlock> caption:RichText = PageBlock;"),
        QStringLiteral("pageBlockChannel#ef1751b5 channel:Chat = PageBlock;"),
        QStringLiteral("pageBlockAudio#31b81a7f audio_id:long caption:RichText = PageBlock;"),
    };

    const QString textData = c_typesSection + QLatin1Char('\n') + sources.join(QLatin1Char('\n')) + QLatin1Char('\n');
    GeneratorNG generator;
    QVERIFY(generator.loadFromText(textData.toLocal8Bit()));
    QMap<QString, TLType> types = generator.types();
    QCOMPARE(types.size(), 1);
    const TLType t = types.first();
    QCOMPARE(t.name, QStringLiteral("TLPageBlock"));
    QCOMPARE(t.subTypes.size(), sources.size());
    for (int i = 0; i < t.subTypes.size(); ++i) {
        const TLSubType &subtype = t.subTypes.at(i);
        QCOMPARE(subtype.source, sources.at(i));
    }
}

QTEST_APPLESS_MAIN(tst_GeneratorNG)

#include "tst_GeneratorNG.moc"
