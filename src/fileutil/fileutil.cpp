#include "fileutil.h"

FileUtil::FileUtil(): QObject(0), msg(new ShowMsg) {

}

/**
 * Create a directory for a given name.
 *
 * @brief WriteFile::writeDir
 */
bool FileUtil::writeDir()
{
    QDir dir = QDir::root(); // Root is equal = "/" or "C:" to Windows
    if (dir.mkdir(getWorkSpace())) {
//        qWarning("%s directory, Created Sucessfully...", getWorkspace().toStdString().c_str());
        return true;
    }
    return false;
}

/**
 * Valide if dir really exist.
 *
 * @brief WriteFile::validDir
 * @return true or false
 */
bool FileUtil::validDir()
{
    QDir dir = QDir::root();
    if (dir.cd(getWorkSpace())) {
        return true;
    }
    return false;
}
/**
 * Valid if path to workspace is
 * a name allowed...
 * @brief WriteFile::validStrDir
 * @return
 */
bool FileUtil::validStrDir(QString &str){
    /**
     * Means:
     * Treat: À to ÿ, à to ÿ, A to Z, a to z, 0 to 9
     * Treat: ., -, _, /
     * '^[' to start, and ']+$' to stop treatments.
     * @brief rx3
     */
    QRegExp mrgx("^[A-Za-z0-9\\.\\-\\_\\/]+$");
    if(!str.contains(mrgx)){
        return false;
    }
    return true;
}


/**
 * Validates that the file name does not
 * contain incompatible characters.
 * @brief WriteFile::validStrFile
 * @return
 */
bool FileUtil::validStrFile(QString &str)
{
    /**
     * All characters different of than those below
     * will not be allowed.
     * Treat: A to Z, a to z, 0 to 9
     * Treat: ., -, _, /
     * '^[' to start, and ']+$' to stop treatments.
     * @brief rx3
     */
    QRegExp mrgx("^[A-Za-z0-9\\.\\-\\_]+$");
    QRegExp reg_txt("*.txt");
    reg_txt.setPatternSyntax(QRegExp::Wildcard);
    QRegExp reg_md("*.md");
    reg_md.setPatternSyntax(QRegExp::Wildcard);
    QRegExp reg_log("*.log");
    reg_log.setPatternSyntax(QRegExp::Wildcard);

    if(!(reg_txt.exactMatch(str)) & !(reg_md.exactMatch(str)) & !(reg_log.exactMatch(str)))
        return false;

    if(!str.contains(mrgx)) return false;
//    qWarning("Past in First test....");

    if(str.count('.') > 1) return false;
//    qWarning("Past in Second test....");

    QString str_ext = str.split(".").at(1).toLower();//get extension
    //
    if(str_ext.size() > 3) return false;
//    qWarning("Past in Third test....");

    if(!(str_ext == "log") && !(str_ext == "txt") && !(str_ext == "md")) {
        return false;
    }

    return true;
}

/**
 * Validates the file name and extension, to check
 * for wrong characters. And also if the extension
 * is incompatible.
 * @brief WriteFile::validStrAudioFileName
 * @return true or false
 */
bool FileUtil::validStrAudioFileName(QString &str)
{
    /**
     * All characters different of than those below
     * will not be allowed.
     * Treat: A to Z, a to z, 0 to 9
     * Treat: ., -, _, /
     * '^[' to start, and ']+$' to stop treatments.
     * @brief rx3
     */
    QRegExp mrgx("^[A-Za-z0-9\\.\\-\\_]+$");
    QRegExp reg_mp3("*.mp3");
    reg_mp3.setPatternSyntax(QRegExp::Wildcard);

    if(!reg_mp3.exactMatch(str))
        return false;

    if(!str.contains(mrgx)) return false;
    if(str.count('.') > 1) return false;

    QString str_ext = str.split(".").at(1).toLower();//get extension
    //
    if(str_ext.size() > 3) return false;

    if(!(str_ext == "mp3")) {
        return false;
    }

    return true;
}

/**
 * Valid if file exist for the given name.
 * @brief FileUtil::validFile
 * @return
 */
bool FileUtil::validFile()
{
    QString path_file = getWorkSpace() + getTextFile()+ getTextFileExt();
    QFile file(path_file);

    if(file.exists())
        return true;
    return false;
}

/**
 * getAudiofile()
 * @brief FileUtil::getAudioFile()
 * @return
 */
QString FileUtil::getAudioFile() const
{
    return audioFile;
}

/**
 * setAudiofile(const QString &newAudiofile)
 * audioFile = path + newAudiofile
 * @brief FileUtil::setAudioFile
 * @param newAudioFile
 */
void FileUtil::setAudioFile(const QString &newAudiofile)
{
    //get file without extension
    QString str_file = newAudiofile.split(QLatin1Char('.')).at(0);

    //get extension
    QString str_ext  = "." +newAudiofile.split(QLatin1Char('.')).at(1);
    setAudioExt(str_ext);

    //Full name with  + workspace + filename
    QString pathfile = getWorkSpace()+str_file;

    audioFile = pathfile;
}

/**
 * getFiletext()
 * @brief FileUtil::getTextFile()
 * @return
 */
QString FileUtil::getTextFile() const
{
    return textFile;
}

/**
 * setTextFile(const QString &newFiletext)
 * setFullflpath() = path + filetext
 * @brief FileUtil::setTextFile(const QString &newFiletext)
 * @param QString &newFiletext
 */
void FileUtil::setTextFile(const QString &newFiletext)
{
    QDateTime dt_time(QDateTime::currentDateTime());
    QString format = "dd-MM-yyyy-HH";
    QString date_to_file  = dt_time.toString(format);

    //get file without extension
    QString str_file = newFiletext.split(QLatin1Char('.')).at(0);

    //get extension
    QString str_ext  = "." +newFiletext.split(QLatin1Char('.')).at(1);
    setTextFileExt(str_ext);

   //Full name with  + workspace + filename + datepart
    QString pathfile = getWorkSpace()+str_file+"-"+date_to_file;

    textFile = pathfile;
}

/**
 * getWorkspace()
 * Defined in the geral menu.
 * @brief FileUtil::getWorkspace
 * @return
 */
QString FileUtil::getWorkSpace() const
{
    return workSpace;
}


/**
 * setWorspace(const QString &newWorkspace)
 * get it from config file.
 * @brief FileUtil::setWorkSpace
 * @param newWorkspace
 */
void FileUtil::setWorkSpace(const QString &newWorkspace)
{
    workSpace = newWorkspace.endsWith('/')? newWorkspace : newWorkspace+'/';
}

QString FileUtil::getTmp_audiofn() const
{
    return tmp_audiofn;
}

void FileUtil::setTmp_audiofn(const QString &newTmp_audiofn)
{
    tmp_audiofn = newTmp_audiofn;
}

QString FileUtil::getTmp_textfn() const
{
    return tmp_textfn;
}

void FileUtil::setTmp_textfn(const QString &newTmp_textfn)
{
    tmp_textfn = newTmp_textfn;
}

ShowMsg *FileUtil::getMsg() const
{
    return msg;
}

void FileUtil::setMsg(ShowMsg *newMsg)
{
    msg = newMsg;
}

QString FileUtil::getAudioExt() const
{
    return audioExt;
}

void FileUtil::setAudioExt(const QString &newAudioExt)
{
    audioExt = newAudioExt;
}

QString FileUtil::getTextFileExt() const
{
    return textFileExt;
}

void FileUtil::setTextFileExt(const QString &newTextFileExt)
{
    textFileExt = newTextFileExt;
}


/**
 * Method to write to txt file.
 * @brief FileUtil::writeTxtFile
 * @param text
 */
void FileUtil::writeTxtFile(const QString &text) {
    //
    QDateTime dt_time(QDateTime::currentDateTime());

    QString format1 = "dd-MM-yyyy HH:mm:ss";
    QString date_formated = dt_time.toString(format1);

    QString file_path = getTextFile() + getTextFileExt();

   // qDebug() << "Actual file path: " << file_path;

    QFile localFile(file_path);

    if (localFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream out(&localFile);
        out << "\n";
        out << "---\n";
        out << "### Data: " << date_formated << "\n";
        out <<"\n" << text << "\n";
        out << "\n---\n";
        localFile.close();

        if(localFile.exists() && localFile.size() > text.size() ){
            msg->setMsgType(INFO);
            msg->setMsg("File Saved Successfully...");
            setMsg(msg);
//            qDebug() << msg->getMsg();
        }
    } else {
        msg->setMsgType(ERROR);
        QString error_msg = "Error, Unable to write data to file: " + file_path;
        msg->setMsg(error_msg);
        setMsg(msg);
//        qDebug() << msg->getMsg();
    }

}

/**
 * Method to write to audio file
 * @brief FileUtil::writeMp3File
 * @param strurl
 * @param oper
 * @param seq
 */
void FileUtil::writeMp3File(const QString &strurl, const QString &oper, int seq)
{
    QString m_seq = seq ==0? "":"-"+QString::number(seq);
    const QString str_file =
        getAudioFile() +   //get audiofile name path + name without extension
        "-"  +             //Just to separator
        oper +             //sigla to lang
        m_seq +            //File sequencia
        getAudioExt();     //get extension with point.
    //
    QFile localFile(str_file);

    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(strurl)));
    QEventLoop event;
    connect(response, SIGNAL(finished()), &event, SLOT(quit()));
    event.exec();

    const QByteArray sdata = response->readAll();
    //
    if(sdata.isEmpty()){
        msg->setMsgType(ERROR);
        QString error = "Error, Unable to get data from url: " + strurl;
        msg->setMsg(error);
        setMsg(msg);
//        qDebug() << msg->getMsg();
        return;
    }

    if (!localFile.open(QIODevice::WriteOnly)){
        msg->setMsgType(ERROR);
        QString error = "Error Opening file: " + str_file;
        msg->setMsg(error);
        setMsg(msg);
//        qDebug() << msg->getMsg();
        return;
    }

    localFile.write(sdata);
    localFile.close();

    if(localFile.exists() && localFile.size() == sdata.size()){
        msg->setMsgType(INFO);
        msg->setMsg("Download Sucessfully...");
        setMsg(msg);
//        qDebug() << msg->getMsg();
    }
}



