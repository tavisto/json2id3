#include <getopt.h>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

/* TagLib */
#include <tag.h>
#include <aifffile.h>
#include <id3v2header.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <textidentificationframe.h>
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

/** Read the tags from a json file
 * @param string file_name
 * return string
 */
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

/** Print usage info
 * @param char **argv
 * return void
 */
void usage(char **argv)
{
    cerr << "Usage: ./" << *argv << " -h --help -f --file [file] -t --tags [file]" << endl;
}

/** Print simple info from the audio file
 * @param string file_name
 * return void
 */
void print_file_info(string file_name)
{
    TagLib::FileRef f(file_name.c_str());
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
    TagLib::ByteVector id(tag_name.c_str(), 4); // Only use the first 4 chars for the id
    cout << "Tagging " << id << endl;

    // Clean up old frames before replacing it
    f->tag()->removeFrames(id); 

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
    stringstream ss;
    ss << tag_value;
    return add_tag(f, tag_name, ss.str());
}

/** Remove all existing frames to ensure only one id3 tag block
 * @param TagLib::RIFF::AIFF::File* f
 * return void
 */
void remove_all_frames(TagLib::RIFF::AIFF::File* f)
{
    const TagLib::ID3v2::FrameList& frameList = f->tag()->frameList();
    for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
         it != frameList.end();) {
        f->tag()->removeFrame(*it++, true);
    }
}

/** Parse all the tags from the json file and tag them into the audio file
 * @parm json::Object* json_obj
 * @param Taglib::RIFF::AIFF::File* f
 * return int
 */
int tag_from_json(json::Object* json_obj,TagLib::RIFF::AIFF::File* f)
{
    for (json::Object::const_iterator i = json_obj->begin(); i != json_obj->end(); ++i) {
        if (i.key() == "tags") {
            json::Object tags_obj = dynamic_cast<const json::Object &>(i.value());
            for (json::Object::const_iterator j = tags_obj.begin(); j != tags_obj.end(); ++j) {
                string tag_name = j.key();
                if (j->type() == json::TYPE_STRING) {
                    string tag_value = dynamic_cast<const json::String &>(*j).value();
                    cout << tag_name << endl;
                    // Special case for comment and genres
                    if (tag_name == "COMM") {
                        f->tag()->setComment(tag_value);
                        cout << "Setting Comment(COM): " << tag_value << endl;
                    }
                    else if (tag_name == "GENR") {
                        f->tag()->setGenre(tag_value);
                        cout << "Setting GENRE: " << tag_value << endl;
                    }
                    else if (add_tag(f, tag_name, tag_value))
                    {
                        cerr << "There was a problem with tag: " << tag_name << " value: " << tag_value << endl;
                        return 1;
                    }
                }
                if (j->type() == json::TYPE_INTEGER) {
                    int tag_value = dynamic_cast<const json::Integer &>(*j).value;
                    cout << tag_name << endl;
                    add_tag(f, tag_name, tag_value);
                }
            }
        }
    }
    f->save();
    return 0;
}


int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    int return_code;
    int option_index = 0;
    string file_name;
    string tag_file;

    if(argc < 2)
    {
        usage(argv);
        return 1;
    }
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
    try {
        json::Value *json_v = json::parse(json_tags);
        json::Object *json_obj = dynamic_cast<json::Object *>(json_v);

        TagLib::RIFF::AIFF::File f(file_name.c_str());
        remove_all_frames(&f);

        return tag_from_json(json_obj, &f);
    }
    catch (runtime_error& e)
    {
        cerr << "Unable to process file for tagging. " << endl;
        return 1;
    }
}
