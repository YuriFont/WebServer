

def build_multipart(boundary, parts):
    body = []

    for part in parts:
        if not part.get("enabled", True):
            continue

        body.append(b"--" + boundary + b"\r\n")

        disp = b'Content-Disposition: form-data; name="' + part["name"] + b'"'
        if part.get("filename"):
            disp += b'; filename="' + part["filename"] + b'"'
        body.append(disp + b"\r\n")

        body.append(b"Content-Type: " + part["type"] + b"\r\n\r\n")
        body.append(part["content"])
        body.append(b"\r\n")

    body.append(b"--" + boundary + b"--\r\n")
    return b"".join(body)