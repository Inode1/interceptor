/*
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <iomanip>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "data_load.h"

using namespace std;
using namespace boost::filesystem;
using namespace boost::iostreams;


const char* DataLoad::m_util[] = { "md5sum ",
                                   "sha1sum ",
                                   "sha256sum "
                                 };
const string DataLoad::m_releaseName = "Release";
const string DataLoad::m_packages    = "Packages";
const string DataLoad::m_ubuntuArchive = "ru.archive.ubuntu.com/ubuntu/dists/";  //ru.archive.ubuntu.com/ubuntu/dists

DataLoad::DataLoad(const std::map<std::string, fullPackageData> &interceptData):
        m_interceptData(interceptData)
{
}
bool DataLoad::Load()
{
    string command = string("wget -nv -R index.html -r --no-parent -A 'Packages.*','Release' > /dev/null 2>&1 ") + m_ubuntuArchive;
    cout << "Wait while all data will be loaded." << endl;
    if (system(command.c_str()))
    {
        cout << "Something happen. Data isn`t loaded." << endl;
        return false;
    }

    cout << "All data load." << endl;
    return true;
}

bool DataLoad::GetFileInfo(const string &fileName, packageInfo &packageData, EPackageInfo info)
{
    if (info == eAllField)
    {
        for (int i= 0; i < eAllField; ++i)
        {
           if (!GetFileInfo(fileName, packageData, static_cast<DataLoad::EPackageInfo>(i)))
               return false;
        }
        return true;
    }
    if (info == eSize)
    {
        path filePath(fileName);

        if (!exists(filePath) && is_directory(filePath))
        {
            cout << "Bad file" << endl;
            return false;
        }
        else
        {
            get<eSize>(packageData) = to_string(file_size(filePath));
            return true;
        }
    }

    string command = string{DataLoad::m_util[info]} + fileName;
    FILE *fp;
    fp =popen(command.c_str(),"r");
    boost::iostreams::stream_buffer<boost::iostreams::file_descriptor_source> fpstream(fileno(fp), boost::iostreams::never_close_handle);
    std::istream in (&fpstream);

    string line;
    if (in)
    {
        getline(in, line);
    }
    /* close */
    pclose(fp);

    if (line.empty())
    {
        return false;
    }
    std::size_t found = line.find(" ");
    if (found == std::string::npos)
    {
      return false;
    }
    line = line.substr(0, found);
    if (info == eMD5Sum)
    {
        get<eMD5Sum>(packageData) = line;
    }
    if (info == eSHA1Sum)
    {
        get<eSHA1Sum>(packageData) = line;
    }
    if (info == eSHA256Sum)
    {
        get<eSHA256Sum>(packageData) = line;
    }
    return true;
}

bool DataLoad::ChangePackageFile(const boost::filesystem::path &packagePath, const string &rootPath, packageMap &result)
{
    string command;
    packageInfo info;
    cout << absolute(packagePath) << " and extension is" << packagePath.extension() << endl;
    if (packagePath.extension().string() == ".bz2")
    {
        command = string("bunzip2 -f > /dev/null 2>&1 ") + packagePath.string();
    }
    else if (packagePath.extension().string() == ".gz")
    {
        command = string("gunzip -f > /dev/null 2>&1 ") + packagePath.string();
    }
    else
    {
        return false;
    }

    cout << "Unzip:" << packagePath << " Start" << endl;
    if (system(command.c_str()))
    {
        cout << "Modified:" << absolute(packagePath) << " Failed" << endl;
        return false;
    }

    cout << "Unzip:" << absolute(packagePath) << " Finished" << endl;

    path filePath(packagePath.parent_path() / m_packages);
    string bz2Package = filePath.string() + ".bz2";
    string gzPackage  = filePath.string() + ".gz";
    if (!exists(filePath))
    {
        cout << absolute(packagePath) << " not exist" << endl;
        return false;
    }
    if (boost::filesystem::is_empty(filePath))
    {
        //restore
        command = string("bzip2 -f -k > /dev/null 2>&1 ") + filePath.string();
        system(command.c_str());
        command = string("gzip -f > /dev/null 2>&1 ") +  filePath.string();
        system(command.c_str());

        GetFileInfo(bz2Package,info);
        result.insert(pair<string, packageInfo>(bz2Package.substr(rootPath.length() + 1), info));

        GetFileInfo(gzPackage,info);
        result.insert(pair<string, packageInfo>(gzPackage.substr(rootPath.length() + 1), info));

        return false;
    }

    mapped_file mapPackege(filePath);
    stream<mapped_file> is(mapPackege, std::ios_base::binary);
    string line;
    bool find = false;
    boost::regex e ("^Package: (.*)");
    while(getline(is, line))
    {
        boost::smatch match;

        if (boost::regex_match(line, match, e) && m_interceptData.find(match[1]) != m_interceptData.end())
        {
            string matchResult = match[1]; //NEED !!!!!
            boost::regex findString("^Size: .*");
            while(getline(is, line))
            {
                if (boost::regex_match(line, findString))
                {
                    is.seekp(-(line.length() + 1), ios_base::cur);
                    // check then length insert the same

                    string size = string{"Size: "} + get<eSize>(m_interceptData[matchResult].second);
                    int diff = line.length() - size.length();
                    is << size                                                            << endl;
                    is << "MD5sum: " << get<eMD5Sum>(m_interceptData[matchResult].second)    << endl;
                    is << "SHA1: "   << get<eSHA1Sum>(m_interceptData[matchResult].second)   << endl;
                    int length;
                    if (diff)
                    {
                        length = is.tellg();
                        getline(is, line);
                        getline(is, line);
                        if (diff > 0)
                        {
                            getline(is, line);
                        }
                        is.seekp(length, ios_base::beg);
                    }
                    is << "SHA256: " << get<eSHA256Sum>(m_interceptData[matchResult].second) << endl;
                    if (diff > 0)
                    {
                        is << line << string(diff, '!') << endl;
                    }
                    else if (diff < 0)
                    {
                        is << line.substr(0, line.length() + diff) << endl;
                    }
                    cout << "Package:" << match[1] << " modified in " <<  absolute(packagePath.parent_path())
                         << " directory" << endl;
                    find = true;
                    break;
                }
            }
        }
    }
    if (!find)
    {
        cout << "Packages not find in " <<  absolute(packagePath.parent_path())
             << " directory" << endl;
    }
    GetFileInfo(filePath.string(),info);
    result.insert(pair<string, packageInfo>(filePath.string().substr(rootPath.length() + 1), info));
    command = string("bzip2 -f -k > /dev/null 2>&1 ") + filePath.string();
    system(command.c_str());
    command = string("gzip -f > /dev/null 2>&1 ") +  filePath.string();
    system(command.c_str());


    GetFileInfo(bz2Package,info);
    result.insert(pair<string, packageInfo>(bz2Package.substr(rootPath.length() + 1), info));

    GetFileInfo(gzPackage,info);
    result.insert(pair<string, packageInfo>(gzPackage.substr(rootPath.length() + 1), info));

    return true;
}
bool DataLoad::ChangeReleaseFile(const boost::filesystem::path& rootPath,packageMap &result)
{
    path releasePath(rootPath / m_releaseName);
    if (!exists(releasePath) || boost::filesystem::is_empty(releasePath))
    {
        cout << absolute(releasePath) << " not exists." << endl;
        return false;
    }

    mapped_file mapRelease(releasePath);
    stream<mapped_file> is(mapRelease, std::ios_base::binary);
    string line;
    // TO-DO:: bad performance.
    cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << absolute(releasePath) << endl;
    string modifiedString;
    int n = 0;
    while(getline(is, line))
    {
        modifiedString = line.substr(line.find_last_of(' ') + 1);
        if (result.find(modifiedString) != result.end())
        {
            is.seekg(-(line.length() + 1), ios_base::cur);
            if ((n / result.size()) == 0)
            {
                is << " " << get<eMD5Sum>(result[modifiedString]) << setw(17) << get<eSize>(result[modifiedString]) << " " << modifiedString << endl;
            }
            if ((n / result.size()) == 1)
            {
                is << " " << get<eSHA1Sum>(result[modifiedString]) << setw(17) << get<eSize>(result[modifiedString]) << " " << modifiedString << endl;
            }
            if ((n / result.size()) == 2)
            {
                is << " " << get<eSHA256Sum>(result[modifiedString]) << setw(17) << get<eSize>(result[modifiedString]) << " " << modifiedString << endl;
            }
            ++n;
        }
    }

    return true;
}

void DataLoad::Worker()
{
    while(1)
    {
        string version;
        {
            boost::lock_guard<boost::mutex> lock(m_mutex);
            if(m_workQueue.empty())
            {
                break;
            }
                version = m_workQueue.front();
                m_workQueue.pop();
        }
        path versionPath(version);
        cout << "Thread: " << boost::this_thread::get_id() << " start" << endl;
        if (!exists(versionPath))
        {
            cout << versionPath << " not exists." << endl;
            break;
        }
        boost::regex findPackages("Packages..*");
        packageMap packageHashs;
        for(boost::filesystem::recursive_directory_iterator file(versionPath);
                file != boost::filesystem::recursive_directory_iterator();
                ++file )
        {
            if (boost::regex_match(file->path().filename().string(), findPackages))
            {
                cout << file->path().string().substr(versionPath.string().length() + 1) << endl;
                ChangePackageFile(file->path().string(), versionPath.string(), packageHashs);
                file.pop();
            }
        }
        if (packageHashs.empty())
        {
            cout << "Nothing change" << endl;
            return;
        }
        for(const auto& a: packageHashs)
        {
            cout << "File Changes: " << a.first << endl;
        }

        ChangeReleaseFile(versionPath, packageHashs);
    }
}

void DataLoad::GiveWorkerJob()
{
    if (m_interceptData.empty())
    {
        cout << "Data for intercept empty. Nothing change" << endl;
        return;
    }
    for (auto &record: m_interceptData)
    {
        packageInfo info;
        if(!GetFileInfo(record.second.first, info))
        {
            cout << "Error: Get file size and hash " << record.second.first << endl;
            return;
        }
        record.second.second = move(info);
    }
    path archivePath(m_ubuntuArchive);
    if (!exists(archivePath) || boost::filesystem::is_empty(archivePath))
    {
        cout << absolute(archivePath) << " not exists." << endl;
        return;
    }

    for(const auto& dir: boost::make_iterator_range(directory_iterator(archivePath), {}))
    {
        m_workQueue.push(dir.path().string());
    }
    for (unsigned int i = 0; i < (boost::thread::hardware_concurrency() / 2u); ++i)
    {
        m_threads.add_thread(new boost::thread(bind(&DataLoad::Worker, this)));
    }
    m_threads.join_all();
}

Config::data Config::ReadConfig(const std::string &is)
{
    using boost::property_tree::ptree;
    using boost::algorithm::equals;
    ptree pt;
    data ans;
    ans.loadRepo = false;
    try
    {
    read_xml(is, pt);

    if (pt.get<string>("Config.LoadRepo") == "yes")
    {
        ans.loadRepo = true;
    }
    for (auto &a :pt.get_child("Config") )
    {
        AddrData f;
        if ((a.first == "Entity"))
        {
            f.serverAddr = a.second.get<string>("Server.<xmlattr>.addr");
            f.clientAddr = a.second.get<string>("Client.<xmlattr>.addr");
            for (auto &b :a.second.get_child("Files") )
            {
                if ((b.first == "subFile"))
                {
                    Substitution sub;
                    sub.to = b.second.get<string>("<xmlattr>.to");
                    sub.what = b.second.get<string>("<xmlattr>.what");
                    f.subFile.push_back(sub);
                }
                if ((b.first == "subPackage"))
                {
                    Substitution sub;
                    sub.to = b.second.get<string>("<xmlattr>.to");
                    sub.what = b.second.get<string>("<xmlattr>.what");
                    f.subPackage.push_back(sub);
                }
            }
        }
        ans.addr.push_back(f);
    }
    }
    catch (std::exception &ex)
    {
        cout << " Config can`t read. " << ex.what();
        exit(0);
    }
    return ans;
}

void Config::GetInterceptPackage(const struct data &info, std::map<std::string,DataLoad::fullPackageData> &interceptData)
{
    for (const auto &a : info.addr)
    {
        for (const auto &b : a.subPackage)
        {
            interceptData.insert(std::map<string,DataLoad::fullPackageData>::value_type(b.what,std::make_pair(b.to,DataLoad::packageInfo{})));
        }
    }
}
void Config::GetMapAllIntercept(const struct data &info, std::map<std::string,std::string> &interceptData)
{
    for (const auto &a : info.addr)
    {
        for (const auto &b : a.subPackage)
        {
            interceptData.insert(std::map<string,string>::value_type(b.what,b.to));
        }
        for (const auto &c : a.subFile)
        {
            interceptData.insert(std::map<string,string>::value_type(c.what,c.to));
        }
    }
}

/*int main()
{
    // first param name Package you wand intercept, second param path file on what you want intercept
    DataLoad intercept("firefox", "mc_4.7.0-1ubuntu2_i386.deb");
    intercept.Load();
    intercept.GiveWorkerJob();
    data erf = reaeed("subst.xml");
    map<string,DataLoad::fullPackageData> interceptData;
    for (const auto &a : erf.addr)
    {
        for (const auto &b : a.sub)
        {
            interceptData.insert(std::map<string,DataLoad::fullPackageData>::value_type(b.what,std::make_pair(b.to,DataLoad::packageInfo{})));
        }
    }
    if (erf.loadRepo)
    {
        DataLoad::Load();
    }
    DataLoad intercept(interceptData);
    intercept.GiveWorkerJob();

    return 0;
}*/
