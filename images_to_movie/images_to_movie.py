import sys
import cv2
import getopt
import glob
import os

def processFolder(path, outputFileName):
    print "Processing folder: " + path
    print "Outputting to: " + outputFileName
    
    if os.path.exists(path) and outputFileName != "":
        fileNames = glob.glob(path + "/*.png")
        if (len(fileNames) > 0):
            firstFrame = cv2.imread(fileNames[0])
            print firstFrame.shape[0:2]
            frameSize = firstFrame.shape[0:2]

            writer = cv2.VideoWriter(outputFileName, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'), 10, frameSize)
            for fileName in fileNames:
                currentFrame = cv2.imread(fileName)
                writer.write(currentFrame)
            
            writer.release()
    else:
        print "Path must not be empty;"

def main(argv):
    folder = ''
    outputFileName = "output.m4v"
    try:
        options, arguments = getopt.getopt(argv, "hi:o:", ["input-folder=", "output-file="])
    except getopt.GetoptError:
        print "Invalid usage. Use python images_to_movie.py -i <input folder> -o <output file>"
        sys.exit(2)

    for option, argument in options:
        print option, argument
        if option in ("-i", "--input-folder"):
            folder = argument
        elif option in ("-o", "--output-file"):
            print "setting output file to: " + argument
            outputFileName = argument

    processFolder(folder, outputFileName)

    
if __name__ == '__main__':
    main(sys.argv[1:])