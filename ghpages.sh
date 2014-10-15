# This is meant to be run by Travis to autogenerate the ghpages on commits to master
if [ "$TRAVIS_BRANCH" = 'master' ] && [ "$TRAVIS_PULL_REQUEST" = 'false' ]; then

# Remove build/test folders
rm -rf ./bin ./src ./js

# Remove packaging files
rm -f emscript.sh reemscript.sh ./bower.json ./package.json CONTRIBUTING.md

# Move dist to root
# As the examples grow more developed, this may need to be migrated (to Jekyll, &c.)
cp -fR ./dist/* ./
rm -rf ./dist

# Rename the index.html
mv ./ghpages.html ./index.html

# Push the ghpages
git add --all
git config user.name "Zach Pomerantz"
git config user.email "zmp@umich.edu"
git commit -m "(docs-autogen) ${TRAVIS_REPO_SLUG}."
git push -fq "https://${TOKEN}:x-oauth-basic@github.com/zzmp/juliusjs.git" HEAD:gh-pages

fi