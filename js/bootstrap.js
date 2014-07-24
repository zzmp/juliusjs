function bootstrap() {
  if (runDependencies) setTimeout(bootstrap, 100);
  else {
    Module.callMain([
      '-dfa',   'voxforge/sample.dfa',
      '-v',     'voxforge/sample.dict',
      '-h',     'voxforge/hmmdefs',
      '-hlist', 'voxforge/tiedlist',
      '-input', 'mic',
      '-realtime', '-quiet'
    ]);
  }
};

bootstrap();
