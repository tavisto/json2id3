#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

/* TagLib */
#include <tag.h>
#include <aifffile.h>
#include <mpegfile.h>
#include <id3v2header.h>
#include <id3v2tag.h>
#include <id3v2frame.h>
#include <commentsframe.h>
#include <attachedpictureframe.h>
#include <textidentificationframe.h>
#include <fileref.h>
#include <tbytevector.h>

/* LibJson */
#include <json/json.h>


static struct option long_options[] = {
    {"help",  no_argument,       0, 'h'},
    {"file",  required_argument, 0, 'f'},
    {"tags",  required_argument, 0, 't'},
    {"media", required_argument, 0, 'm'},
    {0, 0, 0, 0}
};

TagLib::ByteVector read_file_bytes(string file_name) {
    TagLib::ByteVector bv;
    struct stat details;
    stat(file_name.c_str(), &details);
    ifstream input(file_name.c_str());
    if (input.is_open()) {
        char* bytes = new char[details.st_size];
        input.read(bytes, details.st_size);
        bv.setData(bytes, details.st_size);
        delete [] bytes;
    } else {
        cerr << "File not found: " << file_name << endl;
    }
    return bv;
}

/** Read the tags from a json file
 * @param string file_name
 * return string
 */
string read_file(string file_name)
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
    cerr << "Usage: " << *argv << " -h --help -f --file [file] -t --tags [file]" << endl;
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

string get_mime_type(string filename) {
    const char *file_ext = strrchr(filename.c_str(), '.');
    if (strcmp(".png", file_ext) == 0) {
        return "image/png";
    } else {
        return "image/jpeg";
    }
}

int add_image_tag(TagLib::RIFF::AIFF::File* f, string tag_name, string image_file_path, unsigned char image_type) {
    cout << "Setting " << tag_name << endl;
    cout << tag_name << endl;
    f->tag()->removeFrames(TagLib::ByteVector(tag_name.c_str(), 4));
    string mime_type = get_mime_type(image_file_path);
    TagLib::ByteVector image_data = read_file_bytes(image_file_path);
    cout << "Mime type: " << mime_type << endl;
    cout << "Image size: " << image_data.size() << " bytes" << endl;

    TagLib::ID3v2::Frame* frame = new TagLib::ID3v2::AttachedPictureFrame();
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setPicture(image_data);
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setMimeType(mime_type);
    if (image_type == TagLib::ID3v2::AttachedPictureFrame::FrontCover) {
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
    } else {
        cerr << "Unknown image type id: " << image_type << endl;
        return 0;
    }
    if (!frame) {
        cerr << "Image frame blew up!" << endl;
        return 0;
    }
    f->tag()->addFrame(frame);
    return 1;
}

/*!
 * Overloaded add_image_tag for MP3 support.
 */
int add_image_tag(TagLib::MPEG::File* f, string tag_name, string image_file_path, unsigned char image_type) {
    cout << "Setting " << tag_name << endl;
    cout << tag_name << endl;
    f->ID3v2Tag()->removeFrames(TagLib::ByteVector(tag_name.c_str(), 4));
    string mime_type = get_mime_type(image_file_path);
    TagLib::ByteVector image_data = read_file_bytes(image_file_path);
    cout << "Mime type: " << mime_type << endl;
    cout << "Image size: " << image_data.size() << " bytes" << endl;

    TagLib::ID3v2::Frame* frame = new TagLib::ID3v2::AttachedPictureFrame();
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setPicture(image_data);
    static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setMimeType(mime_type);
    if (image_type == TagLib::ID3v2::AttachedPictureFrame::FrontCover) {
        static_cast<TagLib::ID3v2::AttachedPictureFrame*>(frame)->setType(TagLib::ID3v2::AttachedPictureFrame::FrontCover);
    } else {
        cerr << "Unknown image type id: " << image_type << endl;
        return 0;
    }
    if (!frame) {
        cerr << "Image frame blew up!" << endl;
        return 0;
    }
    f->ID3v2Tag()->addFrame(frame);
    return 1;
}

// ------------------- add_tag overloads ----------------------- //

int add_tag(TagLib::RIFF::AIFF::File* f, string tag_name, string tag_value)
{
    TagLib::ByteVector id(tag_name.c_str(), 4); // Only use the first 4 chars for the id
    cout << "Tagging " << id << endl;

    // Clean up old frames before replacing it
    f->tag()->removeFrames(id);

    TagLib::ID3v2::Frame* frame;
    frame = new TagLib::ID3v2::TextIdentificationFrame(id, TagLib::String::UTF8);
    if( !frame)
    {
        cerr << "Frame blew up!" << endl;
        return 0;
    }
    frame->setText(tag_value);
    f->tag()->addFrame(frame);
    return 1;
}

int add_tag(TagLib::RIFF::AIFF::File* f, string tag_name, int tag_value)
{
    stringstream ss;
    ss << tag_value;
    return add_tag(f, tag_name, ss.str());
}

int add_tag(TagLib::MPEG::File* f, string tag_name, string tag_value)
{
    TagLib::ByteVector id(tag_name.c_str(), 4); // Only use the first 4 chars for the id
    cout << "Tagging " << id << endl;

    // Clean up old frames before replacing it
    f->ID3v2Tag()->removeFrames(id);

    TagLib::ID3v2::Frame* frame;
    frame = new TagLib::ID3v2::TextIdentificationFrame(id, TagLib::String::UTF8);
    if( !frame)
    {
        cerr << "Frame blew up!" << endl;
        return 0;
    }
    frame->setText(tag_value);
    f->ID3v2Tag()->addFrame(frame);
    return 1;
}

int add_tag(TagLib::MPEG::File* f, string tag_name, int tag_value)
{
    stringstream ss;
    ss << tag_value;
    return add_tag(f, tag_name, ss.str());
}

// ------------------ remove_frames overloads ----------- //

/** Remove all existing frames to ensure only one id3 tag block
 * @param TagLib::RIFF::AIFF::File* f
 * return void
 */
void remove_all_frames(TagLib::RIFF::AIFF::File* f)
{
    // No helper method for RIFF/AIFF to remove frames, so we'll just
    // have to iterate over them all and remove them manually.
    const TagLib::ID3v2::FrameList& frameList = f->tag()->frameList();
    for (TagLib::ID3v2::FrameList::ConstIterator it = frameList.begin();
         it != frameList.end();) {
        f->tag()->removeFrame(*it++, true);
    }
}

/** Remove all existing frames to ensure only one id3 tag block
 * @param TagLib::MPEG::File* f
 * return void
 */
void remove_all_frames(TagLib::MPEG::File* f)
{
    // Need to remove both ID3v2 and ID3v1 frames for MPEG
    // And, we have a handy helper method to do that for us:
    f->strip();
    
    // Create the ID3v2 tag to avoid segfault?
    f->ID3v2Tag(true);
}



// ------------------- tag_from_json overloads ---------------- //

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
                        // Cannot use setComment wrapper without first setting the
                        // language for the comment frame.
                        TagLib::ID3v2::CommentsFrame *commFrame;
                        commFrame = new TagLib::ID3v2::CommentsFrame(TagLib::String::UTF8);
                        commFrame->setLanguage(TagLib::ByteVector("eng", 3));
                        commFrame->setText(tag_value);

                        f->tag()->addFrame(commFrame);
                        cout << "Setting Comment(COMM): " << tag_value << endl;
                    }
                    else if (tag_name == "GENR") {
                        f->tag()->setGenre(tag_value);
                        cout << "Setting GENRE: " << tag_value << endl;
                    }
                    else if (!add_tag(f, tag_name, tag_value))
                    {
                        cerr << "There was a problem with tag: " << tag_name << " value: " << tag_value << endl;
                        return 0;
                    }
                }
                else if (j->type() == json::TYPE_INTEGER) {
                    int tag_value = dynamic_cast<const json::Integer &>(*j).value;
                    cout << tag_name << endl;
                    add_tag(f, tag_name, tag_value);
                }
                else if (j->type() == json::TYPE_OBJECT) {
                    if (tag_name == "APIC") {
                        json::Object apic = dynamic_cast<const json::Object &>(j.value());
                        unsigned char image_type = dynamic_cast<const json::Integer &>(apic.getValue("type")).value;
                        string image_file_path = dynamic_cast<const json::String &>(apic.getValue("file")).value();
                        string mime_type = get_mime_type(image_file_path);

                        if (!add_image_tag(f, tag_name, image_file_path, image_type)) {
                            cerr << "Image frame blew up!" << endl;
                            return 0;
                        }
                    }
                }
            }
        }
    }
    f->save();
    return 1;
}

/** Parse all the tags from the json file and tag them into the audio file
 * @parm json::Object* json_obj
 * @param Taglib::MPEG::File* f
 * return int
 */
int tag_from_json(json::Object* json_obj,TagLib::MPEG::File* f)
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
                        // Cannot use setComment() wrapper without first setting the
                        // language for the comment frame, so we create the comment frame
                        // instance manually.
                        TagLib::ID3v2::CommentsFrame *commFrame;
                        commFrame = new TagLib::ID3v2::CommentsFrame(TagLib::String::UTF8);
                        commFrame->setLanguage(TagLib::ByteVector("eng", 3));
                        commFrame->setText(tag_value);
                        
                        f->ID3v2Tag()->addFrame(commFrame);
                        cout << "Setting Comment(COM): " << tag_value << endl;
                    }
                    else if (tag_name == "GENR") {
                        f->ID3v2Tag()->setGenre(tag_value);
                        cout << "Setting GENRE: " << tag_value << endl;
                    }
                    else if (!add_tag(f, tag_name, tag_value))
                    {
                        cerr << "There was a problem with tag: " << tag_name << " value: " << tag_value << endl;
                        return 0;
                    }
                }
                else if (j->type() == json::TYPE_INTEGER) {
                    int tag_value = dynamic_cast<const json::Integer &>(*j).value;
                    cout << tag_name << endl;
                    add_tag(f, tag_name, tag_value);
                }
                else if (j->type() == json::TYPE_OBJECT) {
                    if (tag_name == "APIC") {
                        json::Object apic = dynamic_cast<const json::Object &>(j.value());
                        unsigned char image_type = dynamic_cast<const json::Integer &>(apic.getValue("type")).value;
                        string image_file_path = dynamic_cast<const json::String &>(apic.getValue("file")).value();
                        string mime_type = get_mime_type(image_file_path);

                        if (!add_image_tag(f, tag_name, image_file_path, image_type)) {
                            cerr << "Image frame blew up!" << endl;
                            return 0;
                        }
                    }
                }
            }
        }
    }
    f->save();
    return 1;
}



int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    int return_code;
    int option_index = 0;
    string file_name;
    string tag_file;
    string file_ext;

    if(argc < 3)
    {
        usage(argv);
        return 1;
    }
    while(42) {
        return_code = getopt_long(argc, argv, "hf:t:m:", long_options,
                        &option_index);
        if(return_code == -1)
            break;

        switch(return_code) {
            case 'h': /* --help */
                usage(argv);
                return 0;
            case 'f': /* --file */
                cout << "Tagging file: " << optarg << endl;
                file_name = optarg;
                file_ext = file_name.substr(file_name.find_last_of(".") + 1);
                break;
            case 't': /* --tags */
                cout << "Using tag file: " << optarg << endl;
                tag_file = optarg;
                break;
            default: /* ??? */
                usage(argv);
                return 1;
        }
    }

    print_file_info(file_name);
    string json_tags = read_file(tag_file);
    try {
        json::Value *json_v = json::parse(json_tags);
        json::Object *json_obj = dynamic_cast<json::Object *>(json_v);

        // Convert file extension to lower case for string comparison
        int i=0;
        while (file_ext[i]) {
            file_ext[i] = tolower(file_ext[i]);
            i++;
        }
        
        if (file_ext == "mp3") {
            TagLib::MPEG::File f(file_name.c_str());
            remove_all_frames(&f);
            if (tag_from_json(json_obj, &f)) {
                return 0;
            }
        } else if (file_ext == "aif" || file_ext == "aiff") {
            TagLib::RIFF::AIFF::File f(file_name.c_str());
            remove_all_frames(&f);
            if (tag_from_json(json_obj, &f)) {
                return 0;
            }
        } else {
            cerr << "Unrecognized file format: " << file_ext << endl;
            return 1;
        }
    }
    catch (runtime_error& e)
    {
        cerr << "Unable to process file for tagging. " << endl;
    }
    return 1;
}
