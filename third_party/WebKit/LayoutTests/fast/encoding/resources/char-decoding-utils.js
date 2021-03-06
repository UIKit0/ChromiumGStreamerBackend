function decodeText(charsetName, characterSequence)
{
    var req = new XMLHttpRequest;
    req.open('GET', 'data:text/plain,' + characterSequence, false);
    req.overrideMimeType('text/plain; charset="' + charsetName + '"');
    req.send('');
    return req.responseText;
}

function decode(charsetName, characterSequence)
{
    var decodedText = decodeText(charsetName, characterSequence);
    var result = "";
    for (var i = 0; i < decodedText.length; ++i) {
        var code = decodedText.charCodeAt(i).toString(16).toUpperCase();
        if (i)
            result += "/";
        result += "U+" + ("0000" + code).slice(-4);
    }
    return result;
}

function testDecode(charsetName, characterSequence, unicode)
{
    shouldBe("decode('" + charsetName + "', '" + characterSequence + "')", "'" + unicode + "'");
}

function batchTestDecode(inputData)
{
    for (var i in inputData.encodings) {
        for (var j in inputData.encoded)
            testDecode(inputData.encodings[i], inputData.encoded[j], inputData.unicode[j]);
    }
}
