# WacomController
 Teensy 3.6 and 4.x Support using USBHost_t36 library for some Wacom tablets
 And maybe a few others.

Warning this is a WIP, which may never be completed, and may also be migrated into the USBHost_t36 library.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Supported Tablets
-----------------

For the most part this libary is setup to understand a subset of the
Wacom tablets. 

We have also added some support for a couple of the Hunion

There is a data structure which uses the Vendor ID and Product ID to map to 
which tablet type we have. 

With many of these Tablets their HID Report Descriptor is mostly useless, 
they instead intercept the reports and do their own parsing.

Current Table:
```
const WacomController::tablet_info_t WacomController::s_tablets_info[] = {
  {0x056A, 0x27 /*"Wacom Intuos5 touch M"*/, 44704, 27940, 2047, 63, 2, 2, INTUOS5, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,  16 },
  {0x056A, 0xD8 /*"Wacom Bamboo Comic 2FG"*/, 21648, 13700, 1023, 31, 2, 2, BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 2},
  {0x056A, 0x302 /*"Wacom Intuos PT S*/, 15200, 9500, 1023, 31,   2, 2, INTUOSHT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 16},
  {0x256c, 0x006d /* "Huion HS64 and H640P"*/, 32767*2, 32767, 8192, 10, 0, 0, H640P, WACOM_INTUOS_RES, WACOM_INTUOS_RES },
  {0x056A, 0xBA /*"Wacom Intuos4 L"*/, 44704, 27940, 2047, 63, 2, 2, INTUOS4L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES,  16 },
   // Added for 4100, data to be verified.
  {0x056A, 0x374, 15200, 9500, 1023, 31, 0, 0, INTUOS4100,  WACOM_INTUOS_RES, WACOM_INTUOS_RES, 0}
 };
```

So far, the Tablets may return data from three different types of operations on the tablet:

Touch - Where you touch the tablet with one or more fingers.  Not all tablets support this, and how many fingers each supports is tablet specific. 

Pen - Some tablets have Pen support - With these you can retrieve the current pen location.  Some of them also allow you to query the pressure on the tablet.  Likewise some allow you to query the angle of the pen.  Some support knowing if the pen is near the tablet. 

The pen may contain one or more buttons, which you can retrieve, and one one of these may be considered an eraser. 

Side Control buttons - Some/most of these tablets have one or more buttons you can press that are on one or more edges of the tablet.  Likewise some may have a wheel you can turn.

More details to come.