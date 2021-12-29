# Gitt Project(C Language)
### 버전 관리를 가능하게 해주는 Git을 직접 만들어보는 프로젝트
- #### Environment
  - MacBook Pro 2.6 GHz 6코어 Intel Core i7

## Test Usage
  ```
  # Makefile이 있는 gitt folder로 이동
  $ cd gitt
  
  $ make
  gcc -Og    gitt.c main.c hash.c list.c sha1.c blob_tree.c  -lpthread -o gitt
  
  # 생성된 실행파일 gitt을 관리하고자 하는 폴더로 옮겨서 사용
  $ mv gitt [관리하고자 하는 폴더]/gitt
  
  # 해당 폴더로 이동
  $ cd [관리하고자 하는 폴더]
  
  $ ./gitt [명령]
  ```
  <img width="700" src="https://user-images.githubusercontent.com/57051773/147325069-2eca71a5-05f4-4aa9-a2f4-0614e112fe27.png">

## 구현 기능
#### `./gitt init`
  ```
  $ ./gitt add .
  ERROR: 초기화된 gitt저장소가 아닙니다.
  $ ./gitt init
  빈 gitt 저장소로 초기화 하였습니다. (/Users/baeseonghyun/Desktop/gitt_test/.gitt)
  $ ./gitt init
  ERROR: 이미 git 저장소로 설정되어 있습니다.
  ```
- 깃 저장소로 초기화하게 되면 아래와 같이 .gitt 폴더 구조가 생기고 이를 통해서 해당 폴더를 관리할 수 있게 됨
  
  <img width="300" src="https://user-images.githubusercontent.com/57051773/147325470-9da1706e-f5dc-400b-bbd7-d877addb82c1.png"> <img width="300" src="https://user-images.githubusercontent.com/57051773/147325568-8b9a0705-7871-4c5e-bd26-ae950f822516.png"><br><br>

   
   
#### `./gitt add [filename] ..` or `./gitt add .`
- add 실행 전 파일들을 생성<br>
  ```
  $ echo helloworld > temp1.txt
  $ echo helloworld > temp2.txt
  $ echo helloworld2 > temp3.txt
  $ mkdir myfolder
  $ cp temp3.txt myfolder/temp4.txt
  $ ls
  gitt      myfolder  temp1.txt temp2.txt temp3.txt
  ```
  
- 특정 파일들을 staging area에 올림
  ```
  $ ./gitt add temp1.txt temp2.txt
  ```
   - `두 파일의 내용이 같기 때문에, 아래와 같이 object 폴더에 1개의 blob파일만 생긴것을 확인할 수 있음`
   - `또한 index 파일도 새로 생성된 것을 확인할 수 있음`<br><br>
      <img width="700" src="https://user-images.githubusercontent.com/57051773/147328676-ff482c37-76cb-431c-9a87-e3fc096c0931.png">

    
- 또는, 모든 파일을 staging area에 올림
  ```
  $ ./gitt add .
  ```
   - `모든 파일에 대한 blob 파일이 생성된 것을 확인할 수 있음`
   - `모든 파일들을 index 파일이 반영하는 것을 확인할 수 있음`<br><br>
      <img width="600" src="https://user-images.githubusercontent.com/57051773/147329758-3d45347b-aa4b-42b0-8bae-9eb9d97df2e3.png">



#### `./gitt commit [commit message]`
- staging area에 있는 파일들을 commit하고, commit message를 남김
  ```
  $ ./gitt commit "first commit"
  ```
  - `아래와 같이 master에 해당 commit의 hash를 저장한 것을 확인할 수 있음`
  - `commit hash를 통해 commit file을 찾아가 보면 아래와 같이, 해당 commit의 tree에 대한 hash와, commit msg가 저장되어 있는 것을 확인할 수 있음`
  - `tree hash를 통해 tree file을 찾아가 보면, 해당 tree에 속한 blob 파일들과, tree file들의 정보가 저장되어 있는 것을 확인 할 수 있음`<br><br>
  	<img width="900" src="https://user-images.githubusercontent.com/57051773/147627965-f0bcc32c-9b15-45d8-82e8-cf7a112f2dc9.png">
- index 파일의 내용이 최근 커밋과 동일한 상태에서 다시 커밋 실행
  ```
  $ ./gitt commit "second commit"
  ```
  - `최근 커밋과 index 파일의 내용이 동일한 경우, 아래와 같이 오류를 출력하는 것을 확인할 수 있음`<br><br>
  	<img width="600" src="https://user-images.githubusercontent.com/57051773/147628259-e14571d3-df3d-4e35-878f-5e6580af7f4c.png">

- 새로운 파일 만들고 staged area에 올린 이후 두 번째 커밋 실행
  ```
  $ echo helloworld3 > temp4.txt
  $ ./gitt add temp4.txt
  $ ./gitt commit "second commit"
  ```
  - `아래와 같이 master에 새로운 commit의 hash를 저장한 것을 확인할 수 있음`
  - `새로운 commit의 commit file에는 아래와 같이, tree에 대한 hash와, 이전 commit 정보(parent), commit msg가 저장되어 있는 것을 확인할 수 있음`
  - `tree hash를 통해 tree file을 찾아가 보면, 해당 tree에 속한 blob 파일들과, tree file들의 정보가 저장되어 있는 것을 확인 할 수 있음`<br><br>
 	 <img width="900" src="https://user-images.githubusercontent.com/57051773/147628707-ee456ddf-98b5-48eb-bf0f-b5e082e1731e.png">
     
#### `./gitt status`
  - 현재 gitt의 상태를 출력
#### `./gitt branch [branch name]`
  - 새로운 branch를 생성
#### `./gitt checkout [branch name]`
  - 다른 branch로 head를 이동



#### 기타
- 따옴표 처리가 가능한 argument parser
  - [argv_parser](https://github.com/xfile6912/MyLibrary/tree/main/argv_parser)
