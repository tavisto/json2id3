#include <getopt.h>
#include <iostream>
#include <fstream>
using namespace std;

/* TagLib */
#include <aifffile.h>
#include <id3v2header.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <textidentificationframe.h>
#include <tag.h>
#include <fileref.h>
#include <tbytevector.h>

/* LibJson */
#include <json/json.h>


static struct option long_options[] = {
    {"help",  no_argument,       0, 'h'},
    {"file",  required_argument, 0, 'f'},
    {"tags",  required_argument, 0, 't'},
    {0, 0, 0, 0}
};

string read_tag_file(string file_name)
{
    ifstream input(file_name.c_str());
    string line;
    string contents;
    if (input.is_open()) {
        while (input.good()) {
            getline(input, line);
            contents += line;
        }
        input.close();
    } else {
        cerr << "File not found: " << file_name << endl;
    }
    return contents;
}

void usage(char **argv)
{
    cerr << "Usage: ./" << *argv << " -h --help -f --file [file] -t --tags [file]" << endl;
}

void print_file_info(string file_name)
{
    TagLib::RIFF::AIFF::File f(file_name.c_str());
    if(f.tag()) {

        TagLib::Tag *tag = f.tag();

        cout << "-- TAG --" << endl;
        cout << "title   - \"" << tag->title()   << "\"" << endl;
        cout << "artist  - \"" << tag->artist()  << "\"" << endl;
        cout << "album   - \"" << tag->album()   << "\"" << endl;
        cout << "year    - \"" << tag->year()    << "\"" << endl;
        cout << "comment - \"" << tag->comment() << "\"" << endl;
        cout << "track   - \"" << tag->track()   << "\"" << endl;
        cout << "genre   - \"" << tag->genre()   << "\"" << endl;
    }

    if(f.audioProperties()) {

        TagLib::AudioProperties *properties = f.audioProperties();

        cout << "-- AUDIO --" << endl;
        cout << "bitrate     - " << properties->bitrate() << endl;
        cout << "sample rate - " << properties->sampleRate() << endl;
        cout << "channels    - " << properties->channels() << endl;
        cout << "length      - " << properties->length() << endl;
    }

}

int add_tag(TagLib::RIFF::AIFF::File* f,string tag_name, string tag_value)
{
    TagLib::ByteVector id(tag_name.c_str(), 4);
    cout << "Tagging " << id << endl;
    TagLib::ID3v2::Frame* frame;
    frame = new TagLib::ID3v2::TextIdentificationFrame(id, TagLib::String::Latin1);
    if( !frame)
    {
        cerr << "Frame blew up!" << endl;
        return 1;
    }
    frame->setText(tag_value);
    f->tag()->addFrame(frame);
    return 0;
}

int add_tag(TagLib::RIFF::AIFF::File* f,string tag_name, int tag_value)
{
    // Conversion from integer to string before tagging
    char numstr[21]; // Enough to hold all numbers up to 64-bits;
    sprintf(numstr, "%d",tag_value);
    return add_tag(f, tag_name, numstr);
}


int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    int return_code;
    int option_index = 0;
    string file_name;
    string tag_file;

    while(42) {
        return_code = getopt_long(argc, argv, "hf:o:a:", long_options, 
                        &option_index);
        if(return_code == -1)
            break;

        switch(return_code) {
            case 'h': /* --help */
                usage(argv);
                return 0;
            case 'f': /* --file*/
                cout << "Tagging file: " << optarg << endl;
                file_name = optarg;
                break;
            case 't': /* --file*/
                cout << "Using tag file: " << optarg << endl;
                tag_file = optarg;
                break;
            default: /* ??? */
                usage(argv);
                return 1;
        }
    }

    print_file_info(file_name);
    string json_tags = read_tag_file(tag_file);

    json::Value *v = json::parse(json_tags);
    json::Object *obj = dynamic_cast<json::Object *>(v);

    TagLib::RIFF::AIFF::File f(file_name.c_str());

    for (json::Object::const_iterator i = obj->begin(); i != obj->end(); ++i) {
        if (i.key() == "tags") {
            json::Object tags_obj = dynamic_cast<const json::Object &>(i.value());
            for (json::Object::const_iterator j = tags_obj.begin(); j != tags_obj.end(); ++j) {
                string tag_name = j.key();
                if (j->type() == json::TYPE_STRING) {
                    string tag_value = dynamic_cast<const json::String &>(*j).value();
                    cout << tag_name << endl;
                    if (tag_name == "COMM") {
                        f.tag()->setComment(tag_value);
                        cout << "Setting Comment(COM): " << tag_value << endl;
                    }
                    if (tag_name == "GENR") {
                        f.tag()->setGenre(tag_value);
                        cout << "Setting GENRE: " << tag_value << endl;
                    }
                    if (tag_name == "TALB") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Album(TALB): " << tag_value << endl;
                    }
                    if (tag_name == "TPE1") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Artist(TPE1): " << tag_value << endl;
                    }
                    if (tag_name == "TPE4") {
                        cout << "Setting Remixer(TPE4): " << tag_value << endl;
                        add_tag(&f, tag_name, tag_value);
                        cout << "Done Setting Remixer(TPE4): " << tag_value << endl;
                    }
                    if (tag_name == "TIT2") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting TITLE(TIT2): " << tag_value << endl;
                    }
                    if (tag_name == "TPUB") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Publisher(TPUB): " << tag_value << endl;
                    }
                    if (tag_name == "TKEY") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Key(TKEY): " << tag_value << endl;
                    }
                    if (tag_name == "TFLT") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Filetype(TFLT): " << tag_value << endl;
                    }
                    if (tag_name == "TCON") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting Content TYpe(TCON): " << tag_value << endl;
                    }
                }
                if (j->type() == json::TYPE_INTEGER) {
                    int tag_value = dynamic_cast<const json::Integer &>(*j).value;
                    cout << tag_name << endl;
                    if (tag_name == "TDOR") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting release year(TDOR): " << tag_value << endl;
                    }
                    if (tag_name == "TDRC") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting year(TDRC): " << tag_value << endl;
                    }
                    if (tag_name == "TBPM") {
                        cout << "Setting bpm(TBPM): " << tag_value << endl;
                    }
                    if (tag_name == "TRCK") {
                        add_tag(&f, tag_name, tag_value);
                        cout << "Setting track (TRCK): " << tag_value << endl;
                    }
                }
            }
        }
    }

    f.save();

    return 0;
}
