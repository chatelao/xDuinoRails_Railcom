const puppeteer = require('puppeteer');
const path = require('path');

(async () => {
  const browser = await puppeteer.launch({ args: ['--no-sandbox'] });
  const page = await browser.newPage();

  await page.goto('file://' + path.resolve(__dirname, 'test.html'));

  const failedTests = await page.evaluate(() => {
    const failed = [];
    const tests = document.querySelectorAll('.test.fail');
    tests.forEach(test => {
      const testName = test.querySelector('.test-name').innerText;
      failed.push(testName);
    });
    return failed;
  });

  if (failedTests.length > 0) {
    console.error('Tests failed:');
    failedTests.forEach(testName => {
      console.error(`  - ${testName}`);
    });
    process.exit(1);
  } else {
    console.log('All tests passed!');
  }

  await browser.close();
})();
