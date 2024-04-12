#!/usr/bin/python3

### This script takes an input path and an output path as parameters
### It will copy all the videos from the input path and re-encode them to the output path

import os
import sys
import subprocess
import argparse
import shutil
from datetime import datetime
from dateutil import tz
import time
from pathlib import Path

def sizeof_fmt(num, suffix="B"):
    for unit in ("", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi"):
        if abs(num) < 1024.0:
            return f"{num:3.1f}{unit}{suffix}"
        num /= 1024.0
    return f"{num:.1f}Yi{suffix}"

def find_videos(input, filter):
    # Find all the videos from the input path
    videos = []
    for root, dirs, files in os.walk(input):
        if os.path.basename(root) in filter:
            continue

        for file in files:
            if file.lower().endswith(".mp4"):
                video_path = os.path.join(root, file)
                videos.append(video_path)
    return videos

def copy_videos(videos, raw):
    # Copy all the videos to the raw path
    count = 0
    for video in videos:
        filename = os.path.basename(video)
        done_file = os.path.join(raw, ".done", f"{filename}.done")
        raw_video_path = os.path.join(raw, filename)

        if not os.path.exists(raw_video_path) and not os.path.exists(done_file):
            print(f"Copying video {video} {sizeof_fmt(os.path.getsize(video)).rjust(8,' ')} to {raw_video_path}")
            # measure time to copy
            start = time.time()
            shutil.copy2(video, raw_video_path)
            end = time.time()
            print(f"took {end - start:.2f} seconds @ {sizeof_fmt(os.path.getsize(video) / (end - start))}/s")
        else:
            count += 1

    if count > 0:
        print(f"Skipped {count} videos since they already exist in {raw}")


# re-encode videos to x265 high quality
def re_encode_videos(raw, output, keep_raw):
    # Re-encode all the videos to the output path
    for root, dirs, files in os.walk(raw):
        for file in files:
            if file.lower().endswith(".mp4"):
                raw_video_path = os.path.join(root, file)
                tmp_file = os.path.join(root, ".tmp", file)
                done_file = os.path.join(root, ".done", f"{file}.done")
                os.makedirs(os.path.dirname(tmp_file), exist_ok=True)
                os.makedirs(os.path.dirname(done_file), exist_ok=True)
                print(f"Processing video {raw_video_path}")
                try:
                    # extract creation date from raw video with ffprobe:
                    # ffprobe -show_data -hide_banner {filename}
                    ffprobe_output = subprocess.run([
                        "/usr/bin/ffprobe",
                        "-show_data",
                        "-hide_banner",
                        raw_video_path
                    ], capture_output=True).stderr.decode("utf-8").split("\n")
                    creation_time_str = next(line for line in ffprobe_output if "creation_time" in line).split(": ")[1]
                    #2024-04-04T19:10:00.000000Z
                    #to datetime object
                    creation_time_utc = datetime.fromisoformat(creation_time_str)
                    #print(f"Creation time   UTC: {creation_time_utc.strftime('%Y-%m-%d %H:%M:%S')}")
                    creation_time_local = creation_time_utc.astimezone(tz.tzlocal())
                    #print(f"Creation time local: {creation_time_local.strftime('%Y-%m-%d %H:%M:%S')}")
                    creation_time = creation_time_local.strftime('%Y-%m-%d_%H-%M-%S')
                except:
                    print(os.stat(raw_video_path))

                    #stat /media/cnardi/0123-4567/PRIVATE/M4ROOT/CLIP/20240404_C0117.MP4
                    #  File: /media/cnardi/0123-4567/PRIVATE/M4ROOT/CLIP/20240404_C0117.MP4
                    #  Size: 201603414       Blocks: 394240     IO Block: 262144 regular file
                    #Device: 8,1     Inode: 19407       Links: 1
                    #Access: (0755/-rwxr-xr-x)  Uid: ( 1000/  cnardi)   Gid: ( 1000/  cnardi)
                    #Access: 2024-04-04 21:29:06.000000000 +0200
                    #Modify: 2024-04-04 23:28:25.000000000 +0200
                    #Change: 2024-04-04 23:28:25.000000000 +0200
                    # Birth: 2024-04-04 23:28:25.000000000 +0200
                    #
                    #stat /home/cnardi/Pictures/SONY\ ZV-E1_raw/20240404_C0117.MP4
                    #  File: /home/cnardi/Pictures/SONY ZV-E1_raw/20240404_C0117.MP4
                    #  Size: 201603414       Blocks: 393760     IO Block: 4096   regular file
                    #Device: 259,6   Inode: 1607420     Links: 1
                    #Access: (0755/-rwxr-xr-x)  Uid: ( 1000/  cnardi)   Gid: ( 1000/  cnardi)
                    #Access: 2024-04-04 21:29:07.267463100 +0200
                    #Modify: 2024-04-04 23:28:25.000000000 +0200
                    #Change: 2024-04-04 21:29:07.097301400 +0200
                    # Birth: -
                    #
                    # The actual shoot time is 21:28 (Paris UTC+2 DST ON) = 19:28 UTC

                    mtime = datetime.fromtimestamp(os.path.getmtime(raw_video_path))
                    mtime.replace(tzinfo=tz.tzlocal())
                    creation_time = mtime.astimezone(tz.tzutc()).strftime('%Y-%m-%d_%H-%M-%S')

                #print(f"Creation time: {creation_time}")
                creation_date = creation_time.split("_")[0]
                creation_month = creation_date[:-3]

                output_video_folder = os.path.join(output, creation_month, creation_date)
                if not os.path.exists(output_video_folder):
                    os.makedirs(output_video_folder)


                crf="25"
                preset="slow"

                output_filename = f"{os.path.splitext(file)[0]}_{crf}{preset}{os.path.splitext(file)[1]}"
                output_filepath = os.path.join(output_video_folder, output_filename)
                if not os.path.exists(output_filepath) and not os.path.exists(done_file):
                    if os.path.exists(tmp_file):
                        print(f"tmp file {tmp_file} already exists, another process is probably re-encoding this video -> exit")
                        exit(1)
                    print(f"Re-encoding video {raw_video_path} to {output_filepath}")

                    # ffmpeg -i input.mp4 -c:v libx265 -crf 22 -preset slow -c:a aac -b:a 256k output.mp4
                    subprocess.run([
                        "ffmpeg",
                        "-y",
                        "-i", raw_video_path,
                        "-map_metadata", "0",
                        "-c:v", "libx265",
                        "-crf", crf,
                        "-preset", preset,
                        "-c:a", "aac",
                        "-b:a", "256k",
                        tmp_file])
                    os.rename(tmp_file, output_filepath)
                    os.utime(output_filepath, (os.stat(raw_video_path).st_atime, os.stat(raw_video_path).st_mtime))
                    # mark as done
                    Path(done_file).touch()

                    if not keep_raw:
                        # remove raw video
                        os.remove(raw_video_path)
        


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Download and re-encode videos')
    parser.add_argument('--input', type=str, help='Input path containing videos to download', required=True)
    parser.add_argument('--output', type=str, help='Output path to store re-encoded videos', required=True)
    parser.add_argument('--raw', type=str, help='Folder to store raw videos, defaults to output + "_raw"')
    parser.add_argument('--keep_raw', action='store_true', help='Keep raw videos after re-encoding, defaults to False')
    # filter out SUB folder by default
    parser.add_argument('--filter', type=str, help='Comma-separated list of folders to filter out, defaults to "SUB"', default="SUB")
    args = parser.parse_args()

    input = os.path.abspath(args.input)
    output = os.path.abspath(args.output)
    raw = os.path.abspath(args.raw) if args.raw else (output + "_raw")
    keep_raw = args.keep_raw
    filter = args.filter.split(",")
    
    if not os.path.exists(input):
        print("Input path does not exist")
        #sys.exit(1)

    if not os.path.exists(output):
        os.makedirs(output)

    if not os.path.exists(raw):
        os.makedirs(raw)

    videos = find_videos(input, filter)

    copy_videos(videos, raw)
    
    re_encode_videos(raw, output, keep_raw)