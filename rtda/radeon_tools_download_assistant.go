
package main

import (
    "fmt"
    "io"
    "net/http"
    "os"
)

var rtda_version = "1.0.1"
var update_check_api_version = "2.1.0"

func main() {

    argCount := len(os.Args[1:])

    if (argCount == 1 && os.Args[1] == "--version") {
        fmt.Printf("RTDA version: %s\n", rtda_version)
        fmt.Printf("UpdateCheckAPI version: %s\n", update_check_api_version)
	os.Exit(0)
    }

    if (argCount != 2) {
        fmt.Printf("Usage: rtda url local_path\n")
        fmt.Printf("\turl - The url to the file to download\n")
        fmt.Printf("\tlocal_path - The path and filename to save the downloaded file\n")
        os.Exit(1)
    }

    fileUrl := os.Args[1]
    localPath := os.Args[2]

    err := DownloadFile(localPath, fileUrl)
    if (err != nil) {
        panic(err)
    }
}


// DownloadFile will download a url to a local file. It's efficient because it will
// write as it downloads and not load the whole file into memory.
func DownloadFile(filepath string, url string) error {

    // Create the file
    out, err := os.Create(filepath)
    if err != nil {
        return err
    }
    defer out.Close()

    // Get the data
    resp, err := http.Get(url)
    if err != nil {
        return err
    }
    defer resp.Body.Close()

    // Write the body to file
    _, err = io.Copy(out, resp.Body)
    if err != nil {
        return err
    }

    return nil
}
