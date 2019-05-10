import numpy as np
import cv2

class MyVideoCapture:

    def __init__(self, video_source=0):
        self.vid = cv2.VideoCapture(video_source)
        self.video_source = video_source
        if not self.vid.isOpened():
            raise ValueError("Unable to open video source", video_source)

        # Get video source width and height
        self.width = self.vid.get(cv2.CAP_PROP_FRAME_WIDTH)
        self.height = self.vid.get(cv2.CAP_PROP_FRAME_HEIGHT)

    # Release the video source when the object is destroyed
    def __del__(self):
        if self.vid.isOpened():
            self.vid.release()
            #self.window.mainloop()

    def get_frame(self):
        if self.vid.isOpened():
            ret, frame = self.vid.read()
            if ret:
                # Return a boolean success flag and the current frame converted to BGR
                return (ret, frame) #cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            else:
                return (ret, None)
        else:
            return (ret, None)

class ImageProcessing:

    def __init__(self, FLAG_ARRAY=[False, False, False, False, False]):
        self.HOME_BASE_FLAG = FLAG_ARRAY[0]
        self.FERRY_FLAG = FLAG_ARRAY[1]
        self.DROP_OFF_FLAG = FLAG_ARRAY[2]
        self.PICK_UP_FLAG = FLAG_ARRAY[3]
        self.BORDER_FLAG = FLAG_ARRAY[4]

        self.video = MyVideoCapture()
        self.width = int(self.video.width)
        self.height = int(self.video.height)

    def print_flags(self):

        print(f"HOME_BASE_FLAG = {self.HOME_BASE_FLAG}\nFERRY_FLAG = {self.FERRY_FLAG}\nDROP_OFF_FLAG = {self.DROP_OFF_FLAG}\nPICK_UP_FLAG {self.PICK_UP_FLAG}\nBORDER_FLAG {self.BORDER_FLAG}\n")

    def process_images(self):

        while True:
            ret, frame = self.video.get_frame()
            output = frame.copy()

            if self.DROP_OFF_FLAG:
                output, (x,y) = self.detect_drop_off(frame)

            if self.BORDER_FLAG:
                output, (x,y) = self.detect_border(frame)

            cv2.imshow("output", output) #np.hstack([frame, output])) #np.hstack([frame, output]))
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    def detect_drop_off(self, frame):

        blur = cv2.medianBlur(frame,5)
        gray = cv2.cvtColor(blur, cv2.COLOR_BGR2GRAY)
        output = frame.copy()
        circles = cv2.HoughCircles(gray,cv2.HOUGH_GRADIENT,1.2,100)
        x = -1
        y = -1
        # ensure at least some circles were found
        if circles is not None:
        	# convert the (x, y) coordinates and radius of the circles to integers
        	circles = np.round(circles[0, :]).astype("int")

        	# loop over the (x, y) coordinates and radius of the circles
        	for (x, y, r) in circles:
        		# draw the circle in the output image, then draw a rectangle
        		# corresponding to the center of the circle
                #print(f"{x}, {y}")
        		cv2.circle(output, (x, y), r, (0, 255, 0), 4)
        		cv2.rectangle(output, (x - 5, y - 5), (x + 5, y + 5), (0, 128, 255), -1)

        return output, (x,y)

    def detect_border(self, frame):

        ilowH = 21
        ihighH = 39
        ilowS = 157
        ihighS = 255
        ilowV = 111
        ihighV = 210
        x = -1
        y = -1
        width = self.width
        height = self.height

        hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)

        lower_hsv = np.array([ilowH, ilowS, ilowV])
        higher_hsv = np.array([ihighH, ihighS, ihighV])
        mask = cv2.inRange(hsv,lower_hsv,higher_hsv)

        location = cv2.findNonZero(mask)

        if location is not None:
            #total = location[0] * location[1]
            try:
                line = cv2.fitLine(location, cv2.DIST_L2,0,0.01,0.01)
                slope = line[1]/line[0] ;
                y_intercept = line[3] - slope * line[2];
                x_intercept = -1*(y_intercept/slope);
                y1 = slope*0+y_intercept
                y2 = slope*width+y_intercept
                cv2.line(frame,(0,y1),(width,y2),(255,0,0),5)

                slope_perp = -1/slope
                y_int_perp = height/2-slope_perp*width/2
                x_perp = int((y_int_perp-y_intercept) / (slope-slope_perp))
                y_perp = int(slope_perp*x_perp + y_int_perp)
                cv2.line(frame,(int(width/2),int(height/2)),(x_perp,y_perp),(255,0,0),5)
                cv2.rectangle(frame, (x_perp - 5, y_perp - 5), (x_perp + 5, y_perp + 5), (0, 128, 255), -1)
                return frame, (x_perp,y_perp)
            except (OverflowError, ValueError):
                return frame, (x,y)
        else:
            return frame, (x,y)

def check_flags(FLAG_ARRAY):
    print(FLAG_ARRAY)

def main():

    HOME_BASE_FLAG = False
    FERRY_FLAG = False
    DROP_OFF_FLAG = False
    PICK_UP_FLAG = False
    BORDER_FLAG = False
    FLAG_ARRAY = [HOME_BASE_FLAG, FERRY_FLAG, DROP_OFF_FLAG, PICK_UP_FLAG, BORDER_FLAG]

    img_proc = ImageProcessing(FLAG_ARRAY)
    img_proc.print_flags()
    img_proc.process_images()


if __name__=='__main__':
    main()
