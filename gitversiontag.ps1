$content = Get-Content .\Version.h  -Encoding utf8 -TotalCount 1
if ($content -match '#define version "(.*?)".*\/\/(.*)')
{
    $version = $matches[1].Trim()    # "SBMS_1.0.0" 추출
    $comment = $matches[2].Trim()    # "Release Version 1.0.0" 추출
    
    echo "Version: $version"
    echo "Comment: $comment"
    git add -A
    git commit -am $comment
    git tag -a $version -m $comment
    git push origin main --tags
}
else
{
    echo "Version.h 파일을 찾을 수 없습니다."
}