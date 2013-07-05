wav-to-nviz - a program that converts .wav audio files to .nviz visual files of the waveform of the audio
made by Nikola Whallon (https://github.com/nikolawhallon/nviz-project, nikola.whallon@gmail.com)

usage: wav-to-nviz in_file_path out_file_path columns rows frames_per_second color

in_file_path			the path of the .wav file to convert
out_file_path			the path of the .nviz file to create
columns				the columns of the .nviz file to create
rows				the rows of the .nviz file to create
frames_per_second		the framerate of the .nviz file to create
color				the color of the waveform in the .nviz file to create

wav-to-nviz is capable of processing .wav audio files with the following format:

<WAVE-form> → RIFF('WAVE'
                   <fmt-ck>            // Format
                   [<fact-ck>]         // Fact chunk
                   [<cue-ck>]          // Cue points
                   [<playlist-ck>]     // Playlist
                   [<assoc-data-list>] // Associated data list
                   <wave-data> )       // Wave data

this is the WAVE format defined by IBM and Microsoft in August 1991 in the document "Multimedia Programming Interface and Data Specifications 1.0"

optional chunks (indicated by [] brackets above) are skipped, but their chunk id (FOURCC) and chunk size (in bytes) are printed out

note that some .wav files have ID3 sections appended to the end of the file - these are ignored, as wav-to-nviz returns after processing the data chunk
