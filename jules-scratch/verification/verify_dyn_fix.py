
import asyncio
import os
from playwright.async_api import async_playwright, expect

async def main():
    async with async_playwright() as p:
        browser = await p.chromium.launch()
        page = await browser.new_page()
        # Use an absolute path to the HTML file
        file_path = os.path.abspath("tools/index.html")
        await page.goto(f"file://{file_path}")
        await page.locator("#input").fill("2E 93 93 78 E4 B4")
        await page.get_by_role("button", name="Decode").click()
        await page.screenshot(path="jules-scratch/verification/verification.png")
        await browser.close()

asyncio.run(main())
