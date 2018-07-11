# ImageExtractor

ImageExtractor is an integration created for Lightspeed OnSite to download product images. These images will be saved locally and a CSV file will be created that lists the product code and corresponding image filename.

## Instructions

To use this integration you must have a Lightspeed Developer Exchange (LDX) account and already have registered an integration. You will need to supply the User-Agent and Private ID when running this application.

## Usage

Run this application from the command line with the following arguments:

Optional Arguments:

>-h, --help			Show this help message

>-s, --server		IP address of the Lightspeed Server. Defaults to 'localhost' if not specified

>-n, --port			Port Number of the Lightspeed Server. Defaults to '9630' if not specified

Required Arguments:

>-u, --user			Lightspeed Username

>-p, --password		Lightspeed Password

>-a, --useragent	User-Agent

>-x, --xpappid		X-PAPPID
