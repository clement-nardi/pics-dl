#!/usr/bin/python3

from dl_and_re_encode_videos import re_encode_video
import argparse
import os

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Re-encode some videos')
    parser.add_argument('--input', type=str, help='Input path containing videos to re-encode', required=True)
    args = parser.parse_args()

    # walk
    input = os.path.abspath(args.input)
    for root, dirs, files in os.walk(input):
        for file in files:
            ext = os.path.splitext(file)[1]
            if ext.lower() in [".mp4", ".mkv", ".avi"]:
                video_path = os.path.join(root, file)
                print(f"Re-encoding video {video_path}")
                output_path = video_path + "_25slow.mp4"
                if os.path.exists(output_path):
                    print(f"Output file {output_path} already exists -> skip")
                    continue
                re_encode_video(video_path, video_path + "_25slow.mp4", "25", "slow")