#!/usr/bin/env python3
"""
serve_viewer.py — serve o brdf_viewer.html + reference.ppm e abre o browser.
Uso: python3 serve_viewer.py <ppm_path> <viewer_path> [port]
"""

import sys, os, threading, webbrowser, signal
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

PPM_PATH    = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("build/apps/result/reference.ppm")
VIEWER_PATH = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("tools/brdf_viewer.html")
PORT        = int(sys.argv[3]) if len(sys.argv) > 3 else 8765

PPM_PATH    = PPM_PATH.resolve()
VIEWER_PATH = VIEWER_PATH.resolve()

class Handler(BaseHTTPRequestHandler):
    def log_message(self, fmt, *args):
        pass  # silent

    def do_GET(self):
        path = self.path.split('?')[0].lstrip('/')

        if path in ('', 'index.html', 'brdf_viewer.html'):
            self._serve_file(VIEWER_PATH, 'text/html; charset=utf-8')
        elif path == 'reference.ppm':
            self._serve_file(PPM_PATH, 'image/x-portable-pixmap')
        else:
            self.send_error(404)

    def _serve_file(self, fpath, mime):
        if not fpath.exists():
            self.send_response(404)
            self.end_headers()
            self.wfile.write(f"Not found: {fpath}".encode())
            return
        data = fpath.read_bytes()
        self.send_response(200)
        self.send_header('Content-Type', mime)
        self.send_header('Content-Length', str(len(data)))
        self.send_header('Cache-Control', 'no-cache')
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(data)

def main():
    if not VIEWER_PATH.exists():
        print(f"ERRO: viewer não encontrado em {VIEWER_PATH}")
        sys.exit(1)

    ppm_status = "OK" if PPM_PATH.exists() else "nao encontrado (faz 'make' primeiro)"
    print(f"")
    print(f"  BRDF Viewer")
    print(f"  ──────────────────────────────────────")
    print(f"  Viewer : {VIEWER_PATH}")
    print(f"  PPM    : {PPM_PATH}  [{ppm_status}]")
    print(f"  URL    : http://localhost:{PORT}")
    print(f"")
    print(f"  Ctrl+C para parar")
    print(f"")

    server = HTTPServer(('localhost', PORT), Handler)

    def open_browser():
        import time; time.sleep(0.3)
        webbrowser.open(f'http://localhost:{PORT}')
    threading.Thread(target=open_browser, daemon=True).start()

    signal.signal(signal.SIGINT, lambda *_: (server.server_close(), sys.exit(0)))
    server.serve_forever()

if __name__ == '__main__':
    main()