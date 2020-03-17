describe('Element p',  () => {
  it('element p', async () => {
    const p = document.createElement('p');
    p.style.width = '300px';
    p.style.height = '300px';
    p.style.backgroundColor = 'grey';
    p.appendChild(document.createTextNode('This is a paragraph.'))

    document.body.appendChild(p);
    await matchScreenshot();
  });

});
