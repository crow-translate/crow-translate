#include "fileutil.h"

FileUtil::FileUtil(): QObject(0), msg(new ShowMsg) {

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
    if(str.count('.') > 1) return false;

    QString str_ext = str.split(".").at(1).toLower();//get extension
    if(str_ext.size() > 3) return false;

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
    if(str_ext.size() > 3) return false;

    if(!(str_ext == "mp3")) {
        return false;
    }
    return true;
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






