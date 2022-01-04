
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
- 깃 저장소로 초기화하게 되면 아래와 같이 .gitt 폴더 구조가 생기고 이를 통해서 해당 폴더를 관리할 수 있게 됨<br><br>
  <img width="300" src="https://user-images.githubusercontent.com/57051773/147325470-9da1706e-f5dc-400b-bbd7-d877addb82c1.png"> <img width="300" src="https://user-images.githubusercontent.com/57051773/147325568-8b9a0705-7871-4c5e-bd26-ae950f822516.png"><br><br>

   
   
#### `./gitt add [filename] ..` or `./gitt add .`
- add 실행 전 파일들 생성
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
      <img width="600" src="https://user-images.githubusercontent.com/57051773/147329758-3d45347b-aa4b-42b0-8bae-9eb9d97df2e3.png"><br><br>



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
  - `tree hash를 통해 tree file을 찾아가 보면, 아래와 같이 해당 tree에 속한 blob 파일들과, tree file들의 정보가 저장되어 있는 것을 확인 할 수 있음`
  - `만약 checkout을 통해 현재 HEAD에서 브랜치가 아닌 특정 commit hash를 가리키고 있는 경우, HEAD파일에 새로운 commit의 commit hash를 덮어써 줌`<br><br>
 	 <img width="900" src="https://user-images.githubusercontent.com/57051773/147628707-ee456ddf-98b5-48eb-bf0f-b5e082e1731e.png"><br><br>

#### `./gitt branch [branch name]`
- 현재 브랜치 조회
	```
	$ ./gitt branch
	# 현재 branch를 조회
	```
	- `현재 브랜치가 master 브랜치인 것을 확인할 수 있음`<br><br>
		<img width="300" src="https://user-images.githubusercontent.com/57051773/147899988-680a350e-e776-4e96-91af-08e100460706.png">
		
- 새로운 브랜치 생성
	```
	$ ./gitt branch test
	# test branch 생성
	```
	- `아래와 같이 test 브랜치에 대한 파일이 생성되고, 현재 HEAD가 가리키고 있는 브랜치인 master의 commit hash를 해당 파일이 저장하고 있는 것을 확인할 수 있음`
	- `만약 현재 HEAD가 브랜치를 가리키지 않고, 특정 commit hash값을 가지고 있다면, 생성된 새로운 브랜치 파일에 해당 commit hash값을 저장하게 됨`<br><br>
		<img width="900" src="https://user-images.githubusercontent.com/57051773/147900125-cadfe3e9-075d-4b54-9427-4b722431b464.png"><br><br>

#### `./gitt checkout [branch name]`
- 결과를 확인하기 위해, checkout 실행 전 master와 test 브랜치에 차이 만들어 줌
	```
	$ echo helloworld4 > temp5.txt
	$ ./gitt add .
	$ ./gitt commit "master commit"
	# master branch는 새로 생성된 파일인 temp5.txt를 포함하고 있게 됨
	```
	- `아래와 같이 master branch는 새로운 commit을 가리키게 됨`
	- `따라서 master와 test 브랜치는 서로 다른 commit hash를 가리키고 있는 것을 확인할 수 있음`<br><br>
		<img width="900" src="https://user-images.githubusercontent.com/57051773/147900619-7e341106-b294-4556-8cd4-3101d7df6920.png">

 - 다른 브랜치로 head를 이동
	```
	$ ./gitt checkout test
	# test 브랜치로 체크아웃
	```
	- `아래와 같이 HEAD파일이 test 브랜치를 참조하는 것을 확인할 수 있음`
	- `또한 HEAD가 master 브랜치인 위의 사진과 비교하였을 때, master 브랜치에서 생성했던 temp5.txt가 사라진 것을 확인할 수 있음`<br><br>
		<img width="250" src="https://user-images.githubusercontent.com/57051773/147900840-c1fb9f02-c63f-4005-91f1-846f5f29841a.png">
		<img width="900" src="https://user-images.githubusercontent.com/57051773/147900753-1ff76ba5-40bb-4628-9c92-1706d9527d95.png">
   
- 다른 commit hash로 head를 이동
	```
	$ ./gitt checkout f554c219a2153238d7287597d6d5b5a157289596
	# 기존의 test branch에서, master branch가 가리키고 있던 commit의 hash값으로 checkout
	```
	- `아래와 같이 HEAD파일이 해당 commit의 hash값을 가지고 있는 것을 확인할 수 있음`
	- `또한 기존에 test 브랜치로 체크아웃하면서 사라졌던 temp5.txt파일이 복구된 것을 확인할 수 있음`<br><br>
		<img width="550" src="https://user-images.githubusercontent.com/57051773/147900953-84c231e7-e139-4c95-a672-0733550f8db1.png">
		<img width="900" src="https://user-images.githubusercontent.com/57051773/147901017-01e88381-2bac-4043-b34d-fa0a848b56ef.png">
   
- 다시 브랜치를 master 브랜치로 바꾸어줌
	```
	$ ./gitt branch master
	current branch: master
	```
	<br>
#### `./gitt status`
- Working Directory와 Staged Area를 비교하여 상태를 출력
	```
    # 현재 상태는 위의 명령들을 순서대로 실행하여 HEAD가 maseter브랜치를 가리킴
    # master 브랜치는 master commit을 가리키는 상태임 [WD = Working Directory]
    
    $ ./gitt status 		
    # WD와 Staging Area가 차이가 없기 때문에 아무것도 출력되지 않음
    
    $ echo helloworld4 > temp6.txt	
    # temp6.txt를 새로 생성하여 WD와 Staging Area의 차이를 만듦
    
    $ ./gitt status			
    # WD에 temp6.txt가 생성되었다고 출력될 것임

    $ ./gitt add .			
    # temp6.txt를 Staging Area로 넣어줌
    
    $ ./gitt status			
    # 이제 WD와 Staging Area가 차이가 없기 때문에 아무것도 출력되지 않음
    
    $ rm temp6.txt			
    # temp6.txt를 삭제하여 WD와 Staging Area의 차이를 만듦
    
    $ ./gitt status			
    # WD에 temp6.txt가 삭제되었다고 출력될 것임
    
    $ echo helloworld6 > temp6.txt
    # 새로 temp6.txt(hellowrold6)를 생성하여 Staging Area의 temp6.txt(helloworld4)와 파일에 차이를 만듦
    
    $ ./gitt status
    # WD에서 temp6.txt가 수정되었다고 출력될 것임
    ```
    - 아래와 같이 예측한대로 수행된 것을 확인할 수 있음<br><br>
    	<img width="700" src="https://user-images.githubusercontent.com/57051773/148012041-96cd7726-ed44-4488-9502-390ef9cc6aea.png">
        

#### 기타
- 따옴표 처리가 가능한 argument parser
  - [argv_parser](https://github.com/xfile6912/MyLibrary/tree/main/argv_parser)
