import { useRef } from 'react'
import { 
	Button, 
	Modal, 
	ModalOverlay, 
	ModalContent, 
	ModalHeader, 
	ModalCloseButton,
	FormControl,
	FormLabel,
	Input,
	ModalBody,
	ModalFooter,
	useDisclosure,
	useToast
} from '@chakra-ui/react'
import { move } from 'formik'

const StorageActionModal = ({
	showStorageModal,
	setShowStorageModal,
	path,
	getNavigate,
	id,
	actionType
}) => {
	const initialRef = useRef(null)
	const finalRef = useRef(null)
	const toast = useToast();

	let modalTitle;
	let modalInputTitle;
	let modalInputPlaceholder;
	let actionName;
	switch (actionType) {
		case 0:
			modalTitle = "Create a new folder";
			modalInputTitle = "Folder name";
			modalInputPlaceholder = "Folder name";
			actionName = "Create";
			break;
		case 1:
			modalTitle = "Rename";
			modalInputTitle = "New folder name";
			modalInputPlaceholder = "Folder name";
			actionName = "Rename";
			break;
		case 2:
			modalTitle = "Move";
			modalInputTitle = "Move to";
			modalInputPlaceholder = "Path";
			actionName = "Move";
			break;
	}

	const createFolder = () => {
		const bodyJson = {
			path: path,
			name: initialRef.current.value
		}
		const url = `http://${window.location.host}/api/filesystem/folder`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
		}).then(response => {
			if (!response.ok) {
				toast({
					title: 'Error',
					description: "Failed to create folder",
					status: 'error',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				  });
			} else {
				toast({
					title: 'Success',
					description: "Folder created",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				getNavigate();
			}
		})
	}

	const rename = () => {
		const bodyJson = {
			id: id,
			name: initialRef.current.value
		}
		const url = `http://${window.location.host}/api/filesystem/rename`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
		}).then(response => {
			if (!response.ok) {
				toast({
					title: 'Error',
					description: "Failed to rename",
					status: 'error',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
			} else {
				toast({
					title: 'Success',
					description: "Renamed",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				getNavigate();
			}
		})
	}

	const move = () => {
		const bodyJson = {
			id: id,
			old_path: path,
			new_path: initialRef.current.value,
		}
		const url = `http://${window.location.host}/api/filesystem/move`;
		fetch(url, {
			method: 'POST',
			credentials: 'include',
			headers: {
				'Accept': 'application/json',
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(bodyJson)
		}).then(response => {
			if (!response.ok) {
				toast({
					title: 'Error',
					description: "Failed to move",
					status: 'error',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
			} else {
				toast({
					title: 'Success',
					description: "Moved",
					status: 'success',
					duration: 3000,
					isClosable: true,
					position: 'top-center',
				});
				getNavigate();
			}
		})
	}

	const onActionClicked = () => {
		switch (actionType) {
			case 0:
				createFolder();
				break;
			case 1:
				rename();
				break;
			case 2:
				move();
				break;
		}
		setShowStorageModal(false);
	}
  
	return (
	  <>
		<Modal
		  initialFocusRef={initialRef}
		  finalFocusRef={finalRef}
		  isOpen={showStorageModal}
		  onClose={() => setShowStorageModal(false)}
		>
		  <ModalOverlay />
		  <ModalContent>
			<ModalHeader>{modalTitle}</ModalHeader>
			<ModalCloseButton />
			<ModalBody pb={6}>
			  <FormControl>
				<FormLabel>{modalInputTitle}</FormLabel>
				<Input ref={initialRef} placeholder={modalInputPlaceholder} defaultValue={actionType == 2 ? path : ""}/>
			  </FormControl>
			</ModalBody>
			<ModalFooter>
			  <Button colorScheme='blue' mr={3} onClick={onActionClicked}>
				{actionName}
			  </Button>
			  <Button onClick={() => setShowStorageModal(false)}>Cancel</Button>
			</ModalFooter>
		  </ModalContent>
		</Modal>
	  </>
	)
  }

export default StorageActionModal;