
import os
from playwright.sync_api import sync_playwright

def run():
    with sync_playwright() as p:
        browser = p.chromium.launch()
        page = browser.new_page()

        # Get the absolute path to the HTML file
        html_file_path = os.path.abspath('tools/index.html')

        page.goto(f'file://{html_file_path}')

        page.fill('#input', '0x74 0x72 0x6C')
        page.click('#decode')
        page.screenshot(path='jules-scratch/verification/verification.png')
        browser.close()

if __name__ == "__main__":
    run()
